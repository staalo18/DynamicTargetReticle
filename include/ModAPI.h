#pragma once
#include "DTR_API.h"

namespace Messaging
{
	using InterfaceVersion1 = ::DTR_API::IVDTR1;

	class DTRInterface : public InterfaceVersion1
	{
	private:
		DTRInterface() noexcept;
		virtual ~DTRInterface() noexcept;

	public:
		static DTRInterface* GetSingleton() noexcept
		{
			static DTRInterface singleton;
			return std::addressof(singleton);
		}

		// InterfaceVersion1
        virtual unsigned long GetDTRThreadId(void) const noexcept override;
		virtual int GetDTRPluginVersion() const noexcept override;
        virtual bool IsReticleActive() const noexcept override;
        virtual void ShowReticle(bool a_show) const noexcept override;
        virtual RE::Actor* GetCurrentTarget() const noexcept override;

	private:
		unsigned long apiTID = 0;
	};
}
