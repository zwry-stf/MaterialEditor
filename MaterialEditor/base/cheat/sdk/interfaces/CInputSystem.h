#pragma once
#include <stdint.h>
#include <cheat/sdk/sdk_object.h>


class CInputSystem
{
	SDK_CLASS(CInputSystem);
public:
	bool isRelativeMouseMode()
	{
		return *reinterpret_cast<bool*>((uint64_t)this + 0x52);
	}
};