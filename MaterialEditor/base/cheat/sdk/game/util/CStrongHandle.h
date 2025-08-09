#pragma once
#include <util/mem.h>


struct ResourceBinding_t
{
	void* pData; // 0x0
	const char** szName; // 0x8
	MEM_PAD(0x8); // 0x10
	int nRefCount; // 0x18
	MEM_PAD(0x14); // 0x1C
}; // size: 0x30

template <typename T>
class CStrongHandle
{
public:
	CStrongHandle() : pBinding(0) {}

	operator T* () const
	{
		if (pBinding == nullptr)
			return nullptr;

		return static_cast<T*>(pBinding->pData);
	}

	T* operator->() const
	{
		if (pBinding == nullptr)
			return nullptr;

		return static_cast<T*>(pBinding->pData);
	}

	ResourceBinding_t* pBinding = 0;
};