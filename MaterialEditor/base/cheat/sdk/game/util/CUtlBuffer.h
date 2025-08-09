#pragma once
#include <util/mem.h>
#include <init/functions.h>


class CUtlBuffer
{
public:
	inline CUtlBuffer(int a1, int nSize, int a3)
	{
		static auto oUtlBufferInit = functions::get<
			void(__fastcall*)(CUtlBuffer*, int, int, int)>
			("CUtlBuffer::Init");

		oUtlBufferInit(this, a1, nSize, a3);
	}

	inline CUtlBuffer(char* data, int nSize, int flag)
	{
		static auto oUtlBufferInit = functions::get<
			void(__fastcall*)(CUtlBuffer*, char*, int, int)>
			("CUtlBuffer::Init2");

		oUtlBufferInit(this, data, nSize, flag);
	}

	inline void putString(const char* szString)
	{
		static auto oUtlBufferPutString = functions::get<
			void(__fastcall*)(CUtlBuffer*, const char*)>
			("CUtlBuffer::PutString");

		oUtlBufferPutString(this, szString);
	}

	inline void ensureCapacity(int nSize)
	{
		static auto oUtlBufferEnsureCapacity = functions::get<
			void(__fastcall*)(CUtlBuffer*, int)>
			("CUtlBuffer::EnsureCapacity");

		oUtlBufferEnsureCapacity(this, nSize);
	}

private:
	MEM_PAD(0x80);
};