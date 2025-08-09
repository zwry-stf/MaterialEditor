#pragma once
#include <util/mem.h>
#include <cheat/sdk/game/util/CStrongHandle.h>
#include <cheat/sdk/sdk_object.h>


class CResourceHandleUtils
{
	SDK_CLASS(CResourceHandleUtils);
public:
	void deleteResource(const ResourceBinding_t* pBinding)
	{
		mem::CallVFunc<void, 2U>(this, pBinding);
	}
};