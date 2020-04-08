/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Domokos Sarmany
/// @author Simon Smart
/// @author Tiago Quintino

/// @date Jan 2019

#ifndef multio_server_actions_Select_H
#define multio_server_actions_Select_H

#include <iosfwd>
#include <vector>

#include "multio/action/Action.h"

namespace eckit { class Configuration; }

namespace multio {
namespace action {

using message::Message;

class Select : public Action {
public:
    Select(const eckit::Configuration& config);

private:

    bool doExecute(Message& msg) const override;

    void print(std::ostream &os) const override;

    bool isMatched(const Message& msg) const;

    bool matchPlan(const Message& msg) const;

    std::vector<std::string> categories_;
};

}  // namespace action
}  // namespace multio

#endif