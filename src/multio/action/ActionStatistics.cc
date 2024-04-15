
#include "ActionStatistics.h"

namespace multio {
namespace action {

ActionStatistics::ActionStatistics() {}

void ActionStatistics::report(std::ostream& out, const std::string& type, const char* indent) {
    std::string str = "    -- <" + type + "> timing";
    reportTime(out, str.c_str(), actionTiming_, indent);
}

}  // namespace action
}  // namespace multio
