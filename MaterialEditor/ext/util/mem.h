#pragma once
#include <stdint.h>

#define PASTE_IMPL(a, b) a##b
#define PASTE(a, b)      PASTE_IMPL(a, b)
#define MEM_PAD(size) char PASTE(pad, __COUNTER__)[size]

namespace mem
{
    template <typename T, int nIndex, class CBaseClass, typename... Args_t>
    inline T CallVFunc(CBaseClass* thisptr, Args_t... argList)
    {
        using VirtualFn_t = T(__thiscall*)(const void*, decltype(argList)...);
        auto arr = *(VirtualFn_t* const*)thisptr;
#ifdef _DEBUG
        if (!arr)
        {
            __debugbreak();
            return T();
        }
#endif

        auto func = arr[nIndex];
#ifdef _DEBUG
        if (!func)
        {
            __debugbreak();
            return T();
        }
#endif

        return func(thisptr, argList...);
    }

    template <typename T = void*>
    inline T GetVFunc(const void* thisptr, size_t nIndex)
    {
        return (*static_cast<T* const*>(thisptr))[nIndex];
    }

    inline void* resolveRelativeAddress(void* nAddressBytes, uint32_t nRVAOffset, uint32_t nRIPOffset)
    {
        uint32_t nRVA = *reinterpret_cast<uint32_t*>((uintptr_t)nAddressBytes + nRVAOffset);
        uint64_t nRIP = reinterpret_cast<uint64_t>(nAddressBytes) + nRIPOffset;

        return reinterpret_cast<void*>(nRVA + nRIP);
    }
}