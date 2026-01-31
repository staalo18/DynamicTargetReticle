#include "_ts_SKSEFunctions.h"
#include "Hooks.h"
#include "ControlsManager.h"
#include "APIManager.h"
#include "TargetReticleManager.h"
#include "ModAPI.h"

namespace DTR {
    namespace Interface {
        int GetDTRPluginVersion(RE::StaticFunctionTag*) {
            return 1;
        }

        bool DTRFunctions(RE::BSScript::Internal::VirtualMachine * a_vm){
            a_vm->RegisterFunction("GetDTRPluginVersion", "_ts_DTR_SKSEFunctions", GetDTRPluginVersion);
            return true;
        }
    } // namespace Interface
} // namespace DTR

/******************************************************************************************/
void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	// Try requesting APIs at multiple steps to try to work around the SKSE messaging bug
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPostLoad:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPostPostLoad:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
	case SKSE::MessagingInterface::kNewGame:
		APIs::RequestAPIs();
        if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
            log::info("{}: BSInputDeviceManager available", __FUNCTION__);
            input->AddEventSink(&DTR::ControlsManager::GetSingleton());
        } else {
                log::warn("{}: BSInputDeviceManager not available", __FUNCTION__);
        }   
        DTR::TargetReticleManager::GetSingleton().Initialize();    
		break;
	}
}
/******************************************************************************************/
SKSEPluginInfo(
    .Version = Plugin::VERSION,
    .Name = Plugin::NAME,
    .Author = "Staalo",
    .RuntimeCompatibility = SKSE::PluginDeclaration::RuntimeCompatibility(SKSE::VersionIndependence::AddressLibrary),
    .MinimumSKSEVersion = { 2, 2, 3 } // or 0 if you want to support all
)

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    long logLevel = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "LogLevel:Log", "SKSE/Plugins/DynamicTargetReticle.ini", 3L);
    bool isLogLevelValid = true;
    if (logLevel < 0 || logLevel > 6) {
        logLevel = 2L; // info
        isLogLevelValid = false;
    }

	_ts_SKSEFunctions::InitializeLogging(static_cast<spdlog::level::level_enum>(logLevel));
    if (!isLogLevelValid) {
        log::warn("{}: LogLevel in INI file is invalid. Defaulting to info level.", __FUNCTION__);
    }
    log::info("{}: DTR Plugin version: {}", __FUNCTION__, DTR::Interface::GetDTRPluginVersion(nullptr));

    Init(skse);
    auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

    if (!SKSE::GetPapyrusInterface()->Register(DTR::Interface::DTRFunctions)) {
        log::error("{}: Failed to register Papyrus functions.", __FUNCTION__);
        return false;
    } else {
        log::info("{}: Registered Papyrus functions", __FUNCTION__);
    }

    SKSE::AllocTrampoline(64);
    
    log::info("{}: Calling Install Hooks", __FUNCTION__);

    Hooks::Install();

    return true;
}

extern "C" DLLEXPORT void* SKSEAPI RequestPluginAPI(const DTR_API::InterfaceVersion a_interfaceVersion)
{
	auto api = Messaging::DTRInterface::GetSingleton();

	log::info("{} called, InterfaceVersion {}", __FUNCTION__, static_cast<uint8_t>(a_interfaceVersion));

	switch (a_interfaceVersion) {
	case DTR_API::InterfaceVersion::V1:
		log::info("{} returned the API singleton", __FUNCTION__);
		return static_cast<void*>(api);
	}

	log::info("{} requested the wrong interface version", __FUNCTION__);
	return nullptr;
}
