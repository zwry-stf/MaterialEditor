#pragma once
#include <init/signatures.h>
#include <cheat/sdk/sdk_object.h>
#include <init/vtable.h>
#include <util/mem.h>


class CEntityInstance;
class CGameEntitySystem
{
	SDK_CLASS(CGameEntitySystem);
public:
	template <typename T>
	inline T* get(int nIndex)
	{
		return reinterpret_cast<T*>(getEntityByIndex(nIndex));
	}

private:
	void* getEntityByIndex(int index)
	{
		static auto oGetEntityByIndex = signatures::get<
			void* (__thiscall*)(void*, int)>
			("GetEntityByIndex");

		return oGetEntityByIndex(this, index);
	}
};