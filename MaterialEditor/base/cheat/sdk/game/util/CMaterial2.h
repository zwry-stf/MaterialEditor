#pragma once
#include <string.h>
#include <util/mem.h>
#include "CStrongHandle.h"
#include <cheat/sdk/sdk_object.h>


class CMaterial2
{
	SDK_CLASS(CMaterial2);
public:
	MEM_PAD(0x10); // 0x0
	int m_nParameterCount; // 0x10
	MEM_PAD(0x4); // 0x14
	void* m_pParameter; // 0x18
	MEM_PAD(0x3D8); // 0x20
	void* pMaterialMode; // 0x3F8

	const char* getName()
	{
		using function_t = const char* (__thiscall*)(CMaterial2*);
		return (*reinterpret_cast<function_t**>(uint64_t(this)))[0](this);
	}
};