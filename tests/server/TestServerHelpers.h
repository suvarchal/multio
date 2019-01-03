/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef multio_TestServerHelpers_H
#define multio_TestServerHelpers_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpFile.h"
#include "eckit/mpi/Comm.h"

#include "atlas/array.h"
#include "atlas/field/Field.h"
#include "atlas/util/Metadata.h"

#include "multio/server/LocalPlan.h"
#include "multio/server/Message.h"
#include "multio/server/SerialisationHelpers.h"

#include "create_random_data.h"
#include "../TestHelpers.h"

namespace multio {
namespace server {
namespace test {

using atlas::array::make_shape;
using atlas::util::Metadata;
using eckit::mpi::comm;

using TestField = std::pair<std::string, std::vector<double>>;

// Default global values, but each test is allowed to override them
inline size_t& field_size() {
    static size_t val;
    return (!val ? (val = 23) : val);
}

inline size_t& root() {
    static size_t rt;  // = 0 if not set to another value
    return rt;
}

inline void set_metadata(Metadata& metadata, const int level, const int step) {
    metadata.set("field_type", "atm_grid");
    metadata.set("gl_size", field_size());
    metadata.set("levels", level);
    metadata.set("steps", step);
}

// TODO: This function still assumes that MPI is used in the test for the transport layer. We will
// have to change this if want to use it for testing other trnasport layers
inline auto create_single_test_field(const Metadata& metadata, const size_t sz) -> TestField {
    auto name = metadata.get<std::string>("name");
    auto vals = (comm().rank() == root()) ? create_random_data(name, sz) : std::vector<double>(sz);
    comm().broadcast(vals, root());
    auto key = pack_metadata(metadata);
    return std::make_pair(key, vals);
}

inline auto create_atlas_field(const TestField& gl_field) -> atlas::Field {
    auto field = atlas::Field("dummy", atlas::array::DataType("real64"), make_shape(field_size()));
    auto view = atlas::array::make_view<double, 1>(field);
    copy(begin(gl_field.second), end(gl_field.second), view.data());
    return field;
}

inline auto set_up_atlas_test_field(const std::string& name) -> atlas::Field {
    auto metadata = atlas::Field{name, atlas::array::DataType("real64"), make_shape(0)}.metadata();
    set_metadata(metadata, 850, 1);

    auto gl_field = create_single_test_field(metadata, field_size());

    auto field = create_atlas_field(gl_field);
    field.metadata() = metadata;

    return field;
}

inline atlas::Field create_local_field(const TestField& glfl, const std::vector<int>& idxmap,
                                       bool add_map_to_metadata = false) {
    auto field =
        atlas::Field(glfl.first, atlas::array::DataType("real64"), make_shape(idxmap.size()));
    if (add_map_to_metadata) {
        field.metadata().set("mapping", idxmap);
    }
    auto view = atlas::array::make_view<double, 1>(field);
    auto ii = 0;
    for (auto idx : idxmap) {
        view(ii++) = (glfl.second)[static_cast<size_t>(idx)];
    }
    return field;
}

inline auto unpack_local_plan(Message& msg) -> LocalPlan {
    auto meta_size = 0ul;
    msg.read(&meta_size, sizeof(unsigned long));

    auto meta_buf = eckit::Buffer(meta_size);
    msg.read(meta_buf, meta_size);

    auto metadata = atlas::util::Metadata{unpack_metadata(meta_buf)};
    auto local_plan = LocalPlan{metadata.get<std::string>("name")};

    auto data_size = msg.size() - meta_size - sizeof(long);
    std::vector<int> local_map(data_size / sizeof(int));
    local_plan.mapping.resize(data_size / sizeof(int));

    msg.read(local_plan.mapping.data(), data_size);

    return local_plan;
}

inline bool operator==(const LocalPlan& lhs, const LocalPlan& rhs) {
    return (pack_metadata(lhs.metadata) == pack_metadata(rhs.metadata)) &&
           (lhs.mapping == rhs.mapping);
}

}  // namespace server
}  // namespace test
}  // namespace multio


#endif  // multio_TestServerHelpers_H