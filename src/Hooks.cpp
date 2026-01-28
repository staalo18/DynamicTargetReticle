#include "Hooks.h"
#include "_ts_SKSEFunctions.h"
#include "TargetReticleManager.h"

namespace Hooks
{
	void Install()
	{
		log::info("Hooking...");

		MainUpdateHook::Hook();

		log::info("...success");
	}

	void MainUpdateHook::Nullsub()
	{
		_Nullsub();

		DTR::TargetReticleManager::GetSingleton().Update();
	}
} // namespace Hooks
