#pragma once
#include <util/mem.h>
#include "CUtlBuffer.h"
#include <init/signatures.h>
#include <init/functions.h>


struct KV3ID_t
{
	const char* szName;
	uint64_t unk0;
	uint64_t unk1;
};

class CKeyValues3
{
public:
	MEM_PAD(0x100);
	uint64_t uKey;
	void* pValue;
	MEM_PAD(0x8);

	void loadFromBuffer(const char* szString)
	{
		CUtlBuffer buffer(0, static_cast<int>(strlen(szString)) + 10, 1);
		buffer.putString(szString);
		loadKV3(&buffer);
	}

	bool loadKV3(CUtlBuffer* buffer)
	{
		static auto oLoadKeyValues = functions::get<
			bool(__fastcall*)(CKeyValues3*, void*, CUtlBuffer*, KV3ID_t*, void*, void*, void*, void*, const char*, int)>
			("LoadKeyValues");

		KV3ID_t kv3ID = { "generic", 0x41B818518343427E, 0xB5F447C23C0CDF8C };
		return oLoadKeyValues(this, nullptr, buffer, &kv3ID, nullptr, nullptr, nullptr, nullptr, "", 0);
	}

	static CKeyValues3* createMaterialResource()
	{
		static auto oSetTypeKV3 = signatures::get<
			CKeyValues3 * (__fastcall*)(CKeyValues3*, uint32_t, uint32_t)>
			("SetTypeKV3");

		CKeyValues3* pKeyValue = new CKeyValues3[0x10];
		return oSetTypeKV3(pKeyValue, 1U, 6U);
	}
};