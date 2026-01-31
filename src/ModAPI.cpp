#include "ModAPI.h"
#include "TargetReticleManager.h"

Messaging::DTRInterface::DTRInterface() noexcept {
	apiTID = GetCurrentThreadId();
}

Messaging::DTRInterface::~DTRInterface() noexcept {}

unsigned long Messaging::DTRInterface::GetDTRThreadId() const noexcept {
	return apiTID;
}

int Messaging::DTRInterface::GetDTRPluginVersion() const noexcept {
	// Encode version as: major * 10000 + minor * 100 + patch
	return static_cast<int>(Plugin::VERSION[0]) * 10000 + 
	       static_cast<int>(Plugin::VERSION[1]) * 100 + 
	       static_cast<int>(Plugin::VERSION[2]);
}

bool Messaging::DTRInterface::IsReticleActive() const noexcept {
    return DTR::TargetReticleManager::GetSingleton().IsReticleActive();
}

void Messaging::DTRInterface::ShowReticle(bool a_show) const noexcept {
    if (a_show) {
        DTR::TargetReticleManager::GetSingleton().SetReticleMode(DTR::TargetReticleManager::ReticleMode::kOn);
    } else {
        DTR::TargetReticleManager::GetSingleton().SetReticleMode(DTR::TargetReticleManager::ReticleMode::kOff);
    }
}

RE::Actor* Messaging::DTRInterface::GetCurrentTarget() const noexcept {
    return DTR::TargetReticleManager::GetSingleton().GetReticleTarget();
}
