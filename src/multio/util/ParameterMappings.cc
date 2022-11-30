#include "ConfigurationContext.h" // Include header now
#include "ParameterMappings.h"
#include "eckit/exception/Exceptions.h"

namespace multio {
namespace util {

namespace {
const YAMLFile& getParameterMappingConfiguration(const GlobalConfCtx& globalConfCtx) {
    if (globalConfCtx.globalConfig().has("parameter-mappings")) {
        return globalConfCtx.getYAMLFile(([&](){
            try {
                return globalConfCtx.globalConfig().getString("parameter-mappings");
            }
            catch (...) {
                std::throw_with_nested(eckit::Exception("The global key \"parameter-mapping\" is supposed to map to a string. Default: \"parameter-mappings.yaml\"."));
            }
        })());
    }
    else {
        return globalConfCtx.getYAMLFile("parameter-mappings.yaml");
    }
}
}  // namespace

ParameterMappings::ParameterMappings(const GlobalConfCtx& globalConfCtx) :
    globalConfCtx_(globalConfCtx),
    configFile_{getParameterMappingConfiguration(globalConfCtx)},
    mappings_{} {}    
    

const std::vector<message::ParameterMapping>& ParameterMappings::getMappings(const std::string& mapping) const {
    auto search = mappings_.find(mapping);
    if (search != mappings_.end()) {
        return search->second;
    }
    if (configFile_.content.has(mapping)) {
        auto mappingConfig =  configFile_.content.getSubConfiguration(mapping);
        
        // Evaluate source block
        if (!mappingConfig.has("source")) {
            std::ostringstream oss;
            oss << "Parameter mapping \"" << mapping << "\" does not list a \"source\" block" << std::endl;
            throw eckit::Exception(oss.str());
        }
        
        auto sourceConfigBlock =  mappingConfig.getSubConfiguration("source");
        if (!sourceConfigBlock.has("file")) {
            std::ostringstream oss;
            oss << "Source block of parameter mapping \"" << mapping << "\" does not list a \"file\" key, mapping to a YAML file." << std::endl;
            throw eckit::Exception(oss.str());
        }
        auto sourceFname = sourceConfigBlock.getString("file");
        if (!sourceConfigBlock.has("path")) {
            std::ostringstream oss;
            oss << "Source block of parameter mapping \"" << mapping << "\" does not list a \"path\" key, specifing the top level key or path in the file \"" << sourceFname << "\"" << std::endl;
            throw eckit::Exception(oss.str());
        }
        auto sourcePath = sourceConfigBlock.getString("path");
        
        auto sourceConfig = globalConfCtx_.getRelativeYAMLFile(configFile_.path.dirName(), sourceFname);
        if (!sourceConfig.content.has(sourcePath)) {
            std::ostringstream oss;
            oss << "YAML file \"" << sourceFname << "\" for parameter mapping \"" << mapping << "\" does have a top-level path \"" << sourcePath << "\"" << std::endl;
            throw eckit::Exception(oss.str());
        }
        
        std::vector<eckit::LocalConfiguration> sourceList = sourceConfig.content.getSubConfigurations(sourcePath);
        
        // Evaluate mappings block
        if (!mappingConfig.has("mappings")) {
            std::ostringstream oss;
            oss << "Parameter mapping \"" << mapping << "\" does not list a \"mappings\" block" << std::endl;
            throw eckit::Exception(oss.str());
        }
        auto mappingsVector = mappingConfig.getSubConfigurations("mappings");
        std::vector<message::ParameterMapping> v;
        v.reserve(mappingsVector.size());
        
        int mcind = 1;
        for (const auto& mc: mappingsVector) {
            if (!mc.has("match")) {
                std::ostringstream oss;
                oss << "Mapping #" << mcind << " of parameter mapping \"" << mapping << "\" does not list a \"match\" block" << std::endl;
                throw eckit::Exception(oss.str());
            }
            if (!mc.has("map") && !mc.has("optionalMap")) {
                std::ostringstream oss;
                oss << "Mapping #" << mcind << " of parameter mapping \"" << mapping << "\" does not list a \"map\" or \"optionalMap\" block" << std::endl;
                throw eckit::Exception(oss.str());
            }
            auto matchBlock = mc.getSubConfiguration("match");
            auto matchKeys = matchBlock.keys();
            if (matchKeys.size() != 1) {
                std::ostringstream oss;
                oss << "Match block of mapping #" << mcind << " of parameter mapping \"" << mapping << "\" should list exactly one key mapping. Found " << matchKeys.size() << " mappings." << std::endl;
                throw eckit::Exception(oss.str());
            }
            std::string sourceKey = matchKeys[0];
            std::string targetKey = matchBlock.getString(sourceKey);
            
            auto mappings = mc.getSubConfiguration("map");             
            auto optionalMappings = mc.getSubConfiguration("optionalMap");             
            
            auto targetPath = mc.has("targetPath") ? eckit::Optional<std::string>{mc.getString("targetPath")} : eckit::Optional<std::string>{};
            
            v.emplace(v.end(), std::move(sourceKey), std::move(mappings), std::move(optionalMappings), sourceList, std::move(targetKey), std::move(targetPath));
            ++mcind;
        }
        mappings_.emplace(mapping, std::move(v));
        return mappings_.at(mapping);
    }
    
    std::ostringstream oss;
    oss << "No parameter mapping \"" << mapping << "\" found. Available mappings: " << std::endl;
    for(const auto& k: configFile_.content.keys()) {
        oss << "  - " << k << std::endl;
    }
    throw eckit::Exception(oss.str());
}


}  // namespace util
}  // namespace multio
