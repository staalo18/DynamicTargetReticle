#include "ControlsManager.h"
#include "TargetReticleManager.h"

namespace DTR {

    RE::BSEventNotifyControl ControlsManager::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) {

        if (!a_event || RE::UI::GetSingleton()->GameIsPaused()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        for (auto* event = *a_event; event; event = event->next) {
            if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
                auto* buttonEvent = static_cast<RE::ButtonEvent*>(event);
                if (!buttonEvent || !buttonEvent->IsDown()) {
                    continue;
                }
                
                const uint32_t key = buttonEvent->GetIDCode();
                if (key == 11) {
                    TargetReticleManager::GetSingleton().ToggleLockReticle();
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace DTR
