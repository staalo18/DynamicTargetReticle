#include "TargetReticleManager.h"
#include "Offsets.h"
#include "_ts_SKSEFunctions.h"
#include "ControlsManager.h"
#include "APIManager.h"

namespace DTR {
    void TargetReticleManager::DispatchTimelineEvent(uint32_t a_messageType, RE::Actor* a_target) {
        auto* messaging = SKSE::GetMessagingInterface();
        if (messaging) {
            DTR_API::DTRTimelineEventData eventData{ a_target };
            messaging->Dispatch(a_messageType, &eventData, sizeof(eventData), nullptr);
        }
    }


    void TargetReticleManager::Initialize()
    {
        m_isReticleLocked = false;
        m_isWidgetActive = false;
        m_reticleTarget = nullptr;
        
        if (m_isInitialized) {
            log::info("{}: CombatTargetReticle already initialized", __FUNCTION__);
            return;
        }

        if (APIs::TrueHUD) {
            APIs::TrueHUD->LoadCustomWidgets(SKSE::GetPluginHandle(), "IntuitiveDragonRideControl/IDRC_Widgets.swf"sv, [this](TRUEHUD_API::APIResult a_apiResult) {
                if (a_apiResult == TRUEHUD_API::APIResult::OK) {
                    log::info("{}: TrueHUD API: IDRC TargetReticle loaded successfully.", __FUNCTION__);
                    APIs::TrueHUD->RegisterNewWidgetType(SKSE::GetPluginHandle(), 'DTR');
                    this->m_isInitialized = true;
                }
            });
        }
    }

    void TargetReticleManager::SetMaxTargetDistance(float a_maxReticleDistance) {
        m_maxReticleDistance = a_maxReticleDistance;
    }

    void TargetReticleManager::SetDistanceMultiplierSmall(float a_distanceMultiplierSmall) {
        m_distanceMultiplierSmall = a_distanceMultiplierSmall;
    }

    void TargetReticleManager::SetDistanceMultiplierLarge(float a_distanceMultiplierLarge) {
        m_distanceMultiplierLarge = a_distanceMultiplierLarge;
    }

    void TargetReticleManager::SetDistanceMultiplierExtraLarge(float a_distanceMultiplierExtraLarge) {
        m_distanceMultiplierExtraLarge = a_distanceMultiplierExtraLarge;
    }
    void TargetReticleManager::SetMaxTargetScanAngle(float a_maxTargetScanAngle) {
        m_maxTargetScanAngle = a_maxTargetScanAngle;
    }

    void TargetReticleManager::Update()
    {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }

        if (!APIs::TrueHUD) {
            return;
        }

        if (!m_isInitialized) {
            log::warn("{}: TargetReticle not initialized", __FUNCTION__);
            return;
        }

        if (m_reticleMode == ReticleMode::kOff) {
            DisposeReticle(true);
            return;
        }

        RE::Actor* newTarget = nullptr;

        if (m_isReticleLocked) {
            if ( !IsValidTarget(m_reticleTarget) ){
                // release reticle lock
                ToggleLockReticle();

                if (m_reticleTarget) {
                    DispatchTimelineEvent(static_cast<uint32_t>(DTR_API::DTRMessage::kLostTarget), nullptr);
                }

                m_reticleTarget = nullptr;
            }
        } else {

            if (APIs::TrueDirectionalMovementV1 && IsTDMLocked()) {
                auto targetHandle = APIs::TrueDirectionalMovementV1->GetCurrentTarget();
                if (targetHandle) {
                    newTarget = targetHandle.get().get();
                } else {
                    newTarget = nullptr;
                }
            } else {
                newTarget = _ts_SKSEFunctions::GetCrosshairTarget();
            }

            if (!IsValidTarget(newTarget)) {
                newTarget = nullptr;
            }

            if ((!m_isWidgetActive && newTarget) ||
                m_reticleTarget != newTarget)
            {
//                bool arg2 = false;
//                if (newTarget && RE::PlayerCharacter::GetSingleton()->HasLineOfSight(newTarget, arg2)) {
                    m_reticleTarget = newTarget;
//                }
//                else {
//                    m_reticleTarget = nullptr;
//                }

                SetReticleTarget();
                if (m_reticleTarget) {
                    DispatchTimelineEvent(static_cast<uint32_t>(DTR_API::DTRMessage::kFoundTarget), m_reticleTarget);
                } else {
                    DispatchTimelineEvent(static_cast<uint32_t>(DTR_API::DTRMessage::kLostTarget), nullptr);
                }
            }
        }

        UpdateReticleState();
    }

    bool TargetReticleManager::IsTDMLocked() {
        bool isTDMLocked = false;
        if (APIs::TrueDirectionalMovementV1 && APIs::TrueDirectionalMovementV1->GetTargetLockState()) {
            isTDMLocked = true;
        }
        return isTDMLocked;
    }

    bool TargetReticleManager::IsValidTarget(RE::Actor* a_target) const {
        auto player = RE::PlayerCharacter::GetSingleton();
        bool arg2 = false;

        return (a_target && GetTargetPoint(a_target) && 
                (a_target->GetDistance(RE::PlayerCharacter::GetSingleton()) < m_maxReticleDistance * GetDistanceRaceSizeMultiplier(a_target->GetRace())) &&
                !a_target->IsDead(true) &&
                (!m_considerLOS || player->HasLineOfSight(a_target, arg2)));
    }

    bool TargetReticleManager::IsReticleLocked() const {
        return m_isReticleLocked;
    }

    bool TargetReticleManager::IsReticleActive() const {
        return m_reticleMode == ReticleMode::kOn ? true : false;
    }

    void TargetReticleManager::ToggleLockReticle() {
        if (!APIs::TrueHUD) {
            log::info("{}: TrueHUD API not available", __FUNCTION__);
            return;
        }

        if (m_reticleTarget || m_isReticleLocked) {
            m_isReticleLocked = !m_isReticleLocked;
            std::string sMessage = "Zoom " + std::string(m_isReticleLocked ? "locked on " + std::string(m_reticleTarget->GetName()) + "." : "unlocked.");
            RE::DebugNotification(sMessage.c_str());
        } else {
            std::string sMessage = "No zoom target to lock on.";
            RE::DebugNotification(sMessage.c_str());
        }

        UpdateReticleState();
    }

    void TargetReticleManager::UpdateReticleState() {

        if (!m_reticleTarget) {
            DisposeReticle();
            return;
        }
        
        auto widget = m_TargetReticle.lock();
        if (!widget) {
            return;
        }
        
        widget->UpdateState(m_isReticleLocked, IsTDMLocked(), 0);
    }

    void TargetReticleManager::DisposeReticle(bool a_keepTarget) {
        if (!APIs::TrueHUD) {
            log::info("{}: TrueHUD API not available", __FUNCTION__);
            return;
        }

        if (!a_keepTarget) {
            if (m_reticleTarget) {
                DispatchTimelineEvent(static_cast<uint32_t>(DTR_API::DTRMessage::kLostTarget), nullptr);
            }

            m_reticleTarget = nullptr;
        }

        auto widget = m_TargetReticle.lock();
        if (!widget || !m_isWidgetActive) {
            return;
        }

        widget->WidgetReadyToRemove();

        APIs::TrueHUD->RemoveWidget(SKSE::GetPluginHandle(), 'DTR', 0, TRUEHUD_API::WidgetRemovalMode::Normal);
        m_isWidgetActive = false;
    }

    void TargetReticleManager::SetReticleMode(ReticleMode a_mode) {

        if (m_reticleMode == kOff && a_mode == kOn && m_reticleTarget) {
           SetReticleTarget();
        }

        m_reticleMode = a_mode; 
    }


    void TargetReticleManager::SetReticleLockAnimationStyle(int a_style) {
        m_reticleLockAnimationStyle = a_style;
        auto widget = m_TargetReticle.lock();
        if (widget) {
            widget->SetReticleLockAnimationStyle(m_reticleLockAnimationStyle);
        }
    }

    void TargetReticleManager::SetReticleTarget() {
        if (!APIs::TrueHUD) {
            log::info("{}: TrueHUD API not available", __FUNCTION__);
            return;
        }

        if (!m_reticleTarget) {
            DisposeReticle();
            return;
        }

        auto actorHandle = m_reticleTarget->GetHandle();
        if (!actorHandle) {
            log::warn("{}: Actor handle is invalid", __FUNCTION__);
            DisposeReticle();
            return;
        }

        auto targetPoint = GetTargetPoint(m_reticleTarget);
        if (!targetPoint) {
            log::warn("{}: Target point is nullptr", __FUNCTION__);
            DisposeReticle();
            return;
        }

        auto widget = m_TargetReticle.lock();
        if (widget) {
            widget->ChangeTarget(actorHandle, targetPoint);
        } else {
            widget = std::make_shared<TargetReticle>(actorHandle.native_handle(), actorHandle, targetPoint,
                                                            m_reticleLockAnimationStyle);
            if (!widget) {
                log::warn("{}: Failed to create TargetReticle widget", __FUNCTION__);
                return;
            }
            
            m_TargetReticle = widget;
            APIs::TrueHUD->AddWidget(SKSE::GetPluginHandle(), 'DTR', 0, "IDRC_TargetReticle", widget);
            }
        m_isWidgetActive = true;

        UpdateReticleState();
    }

    RE::Actor* TargetReticleManager::GetReticleTarget() const {
        return m_reticleTarget;
    }

    RE::NiPointer<RE::NiAVObject> TargetReticleManager::GetTargetPoint(RE::Actor* a_actor) const {
        RE::NiPointer<RE::NiAVObject> targetPoint = nullptr;

        if (!a_actor) {
            return nullptr;
        }

        auto race = a_actor->GetRace();
        if (!race) {
            return nullptr;
        }

        RE::BGSBodyPartData* bodyPartData = race->bodyPartData;
        if (!bodyPartData) {
            return nullptr;
        }

        auto actor3D = a_actor->Get3D2();
        if (!actor3D) {
            return nullptr;
        }
    
        RE::BGSBodyPart* bodyPart = bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kHead];
        if (!bodyPart) {
            bodyPart = bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kTotal];
        }
        if (bodyPart) {
            targetPoint = RE::NiPointer<RE::NiAVObject>(NiAVObject_LookupBoneNodeByName(actor3D, bodyPart->targetName, true));
        }

        return targetPoint;
    }

    float TargetReticleManager::GetDistanceRaceSizeMultiplier(RE::TESRace* a_race) const {
        if (a_race) {
            switch (a_race->data.raceSize.get())
            {
            case RE::RACE_SIZE::kMedium:
            default:
                return 1.f;
            case RE::RACE_SIZE::kSmall:
                return m_distanceMultiplierSmall;
            case RE::RACE_SIZE::kLarge:
                return m_distanceMultiplierLarge;
            case RE::RACE_SIZE::kExtraLarge:
                return m_distanceMultiplierExtraLarge;
            }
        }

        return 1.f;
    }
}  // namespace DTR
