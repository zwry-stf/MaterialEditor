#pragma once
#include <stdint.h>
#include "../../interfaces/CGameResourceService.h"
#include <init/interfaces.h>

#define INVALID_EHANDLE_INDEX 0xFFFFFFFF
#define ENT_ENTRY_MASK 0x7FFF
#define NUM_SERIAL_NUM_SHIFT_BITS 15



class CEntityInstance;

template <typename T = CEntityInstance>
class CHandle
{
public:
	uint32_t nIndex;

public:
	inline CHandle() :
		nIndex(INVALID_EHANDLE_INDEX)
	{
	}

	inline CHandle(const int nEntry, const int nSerial)
		: nIndex(nEntry | (nSerial << NUM_SERIAL_NUM_SHIFT_BITS))
	{
	}

	template <typename U>
	inline CHandle(const CHandle<U>& other)
	{
		nIndex = other.nIndex;
	}

	template <typename U>
	inline CHandle<T> operator=(const CHandle<U> other)
	{
		nIndex = other.nIndex;
		return *this;
	}

	inline CHandle<T> operator=(const int other)
	{
		nIndex = other;
		return *this;
	}

	inline operator bool() const
	{
		return isValid();
	}

	template <typename U>
	inline bool operator!=(const CHandle<U> other) const
	{
		return nIndex != other.nIndex;
	}

	template <typename U>
	inline bool operator==(const CHandle<U> other) const
	{
		return nIndex == other.nIndex;
	}

	template <typename U>
	inline bool operator<(const CHandle<U> other) const
	{
		return nIndex < other.nIndex;
	}

	inline bool operator!() const
	{
		return !isValid();
	}

	inline bool isValid() const
	{
		return nIndex != INVALID_EHANDLE_INDEX;
	}

	inline int getEntryIndex() const
	{
		return static_cast<int>(nIndex & ENT_ENTRY_MASK);
	}

	inline int getSerialNumber() const
	{
		return static_cast<int>(nIndex >> NUM_SERIAL_NUM_SHIFT_BITS);
	}

private:
	inline CEntityInstance* getEntity(int nEntryIndex) const
	{
		return interfaces::iGameResourceService->pGameEntitySystem->get<CEntityInstance>(nEntryIndex);
	}
	inline CEntityInstance* getEntity() const
	{
		return getEntity(getEntryIndex());
	}

public:
	template <typename U = T>
	inline U* get() const
	{
		return reinterpret_cast<U*>(getEntity());
	}
};