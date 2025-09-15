#pragma once
#include <util/Mem.h>
#include "CResourceHandleUtils.h"
#include "../game/util/CBufferString.h"
#include "../game/util/fnv1a.h"
#include "../game/util/CKeyValues3.h"
#include "../game/util/CMaterial2.h"
#include <init/signatures.h>


class CVSoundTypeManager;

class CResourceSystem
{
    SDK_CLASS(CResourceSystem);
public:
    void* QueryInterface(const char* szInterfaceName) const
    {
        return mem::CallVFunc<void*, 2U>(this, szInterfaceName);
    }

    uint32_t ResourceLoaded(CBufferString* buffer)
    {
        return mem::CallVFunc<uint32_t, 55>(this, buffer);
    }

    ResourceBinding_t* getResource(CBufferString* buffer, uint64_t unk) const
    {
        return mem::CallVFunc<ResourceBinding_t*, 80>(this, buffer, unk);
    }

    ResourceBinding_t* loadResource(CBufferString* buffer, const char* load_function = "")
    {
        return mem::CallVFunc<ResourceBinding_t*, 45>(this, buffer, load_function);
    }

    ResourceBinding_t* preCache(CBufferString* buffer)
    {
        return mem::CallVFunc<ResourceBinding_t*, 46>(this, buffer, "");
    }

    ResourceBinding_t* getOrLoad(CBufferString* buffer)
    {
        int handle = ResourceLoaded(buffer);
        if (handle)
            return getResource(buffer, 0LL);

        return loadResource(buffer);
    }

    static CStrongHandle<CMaterial2> createMaterial(const char* szMaterialName, const char szVmatBuffer[])
    {
        static auto oCreateMaterial = signatures::get<
            void* (__fastcall*)(void*, void*, const char*, void*, unsigned int, unsigned int)>
            ("CreateMaterial");

        CKeyValues3 key_values;
        key_values.create();

        if (!key_values.loadFromBuffer(szVmatBuffer))
            return {};

        CStrongHandle<CMaterial2> pCustomMaterial = {};
        void* ret = oCreateMaterial(nullptr, &pCustomMaterial, szMaterialName, &key_values, 0, 1);
        if (!ret)
            return {};

        return pCustomMaterial;
    }
};