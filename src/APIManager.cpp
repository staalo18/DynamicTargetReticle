#include "APIManager.h"

void APIs::RequestAPIs()
{
	if (!TrueDirectionalMovementV1) {
		TrueDirectionalMovementV1 = reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1));
		if (TrueDirectionalMovementV1) {
			log::info("Obtained TrueDirectionalMovement API (V1) - {0:x}", reinterpret_cast<uintptr_t>(TrueDirectionalMovementV1));
		}
	}

    if (!TrueHUD) {
		TrueHUD = reinterpret_cast<TRUEHUD_API::IVTrueHUD3*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
		if (TrueHUD) {
			log::info("Obtained TrueHUD API - {0:x}", reinterpret_cast<uintptr_t>(TrueHUD));
		} else {
			log::info("Failed to obtain TrueHUD API");
		}
	}
}

