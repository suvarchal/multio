
#include "PlanAssembler.h"

#include <fstream>
#include <functional>

#include "eckit/io/Buffer.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"

#include "atlas/util/Metadata.h"

#include "multio/DataSink.h"

#include "multio/server/Action.h"
#include "multio/server/Aggregation.h"
#include "multio/server/Select.h"
#include "multio/server/Sink.h"

#include "multio/server/Message.h"
#include "multio/server/Plan.h"
#include "multio/server/PlanConfigurations.h"
#include "multio/server/SerialisationHelpers.h"

namespace multio {
namespace server {

PlanAssembler::PlanAssembler() : planConfigs_{plan_configurations()} {}

Plan PlanAssembler::createPlan(const atlas::util::Metadata& config) {
    // Each plan has 'Select' as its first action
    auto name = config.getString("name");
    auto categories = config.get<std::vector<std::string>>("categories");
    std::unique_ptr<Action> root{new Select{categories}};
    auto it = root.get();

    auto actionConfig = atlas::util::Metadata{config.getSubConfiguration("actions")};

    // Add aggregation action if need be
    if (actionConfig.get<std::string>("aggregation") == "indexed") {
        it = it->add(std::unique_ptr<Action>{new Aggregation{"Indexed aggregation"}});
    }

    // Add encoding action if need be
    if (actionConfig.get<std::string>("encoding") != "none") {
        ASSERT(false);  // Not yet implemented
    }

    // Add sink action if need be
    if (actionConfig.get<std::string>("multio_sink") == "file") {
        eckit::LocalConfiguration cfg;
        cfg.set("path", eckit::PathName{"/dev/null"});  // Default to black hole
        it = it->add(
            std::unique_ptr<Action>{new Sink{DataSinkFactory::instance().build("file", cfg)}});
    }

    return Plan{name, std::move(root)};
}

std::set<Plan> PlanAssembler::createAllPlans() {
    std::set<Plan> plans;
    auto configs = planConfigs_.getSubConfigurations("plans");
    for (const auto& config : configs) {
        std::string name;
        config.get("name", name);
        plans.insert(createPlan(atlas::util::Metadata{config}));
    }
    return plans;
}

}  // namespace server
}  // namespace multio
