#include "signatures.h"

#include <Windows.h>
#include <stdexcept>
#include <assert.h>
#include "vtable.h"
#include <util/log.h>


struct LoadedSig
{
	std::string name;
	void* address;
};


inline static std::vector<LoadedSig>& loaded_signatures()
{
	static std::vector<LoadedSig>* sigs = new std::vector<LoadedSig>(); // Ensure this still works with RAII
	return *sigs;
}


void* signatures::patternScan(void* start, void* end, const std::vector<int16_t>& pattern, int pre_offset, int post_offset)
{
	uint64_t rangeEnd = (uint64_t)end - pattern.size();
	for (uint64_t Address = (uint64_t)start; Address <= rangeEnd; ++Address)
	{
		bool Match = true;
		for (size_t i = 0; i < pattern.size(); ++i)
		{
			if (pattern[i] == -1) continue;
			if (*(uint8_t*)(Address + i) != pattern[i])
			{
				Match = false;
				break;
			}
		}

		if (Match)
		{
			// Relative Address
			if (pre_offset != 0)
			{
				Address += pre_offset;
				Address += sizeof(int32_t) + *reinterpret_cast<int32_t*>(Address);
			}
			Address += post_offset;

			// Save
			return (void*)Address;
		}
	}

	return nullptr;
}

void* signatures::patternScan(const std::string& module, const std::vector<int16_t>& pattern, int pre_offset, int post_offset)
{
	// Get Module
	uint64_t mod = (uint64_t)GetModuleHandleA(module.c_str());
	if (!mod)
		return nullptr;

	auto DosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(mod);
	auto NTHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(mod + DosHeader->e_lfanew);
	DWORD size = NTHeaders->OptionalHeader.SizeOfImage;

	return patternScan((void*)mod, (void*)(mod + size), pattern, pre_offset, post_offset);
}

void signatures::addSigEx(const std::string& name, const std::string& module, const SignatureData& sig, void** out, int post_offset)
{
	std::vector<int16_t> pattern;
	for (size_t i = 0; i < sig.nLength; i++)
	{
		int16_t c = sig.pData[i];
		pattern.push_back(c ^ sig.nKey);
	}

	void* address = patternScan(module, pattern, sig.nPreOffset, post_offset);

	if (!address)
	{
		log::println("failed to find signature '%s'", name.c_str());

#ifdef NDEBUG
		throw std::runtime_error("Failed to find signatures");
#else
		failed = true;
		return;
#endif
	}

	log::println("signature '%s' found at %p", name.c_str(), address);

	if (out)
		*out = address;

	loaded_signatures().emplace_back(name, address);
}

void signatures::addSigOffsetEx(const std::string& name, const std::string& module, const SignatureData& sig)
{
	std::vector<int16_t> pattern;
	for (size_t i = 0; i < sig.nLength; i++)
	{
		int16_t c = sig.pData[i];
		pattern.push_back(c ^ sig.nKey);
	}

	void* ret = patternScan(module, pattern, 0, 0);
	if (!ret)
	{
		log::println("failed to find signature '%s'", name.c_str());

#ifdef NDEBUG
		throw std::runtime_error("Failed to find signatures");
#else
		failed = true;
		return;
#endif
	}

	int32_t offset = *(int32_t*)((uint64_t)ret + sig.nPreOffset);

	void* value = 0;
	memcpy(&value, &offset, sizeof(offset));

	log::println("signature offset '%s' found at %p", name.c_str(), value);

	loaded_signatures().emplace_back(name, value);
}

void signatures::getSigEx(const std::string& name, void** out)
{
	*out = nullptr;

	for (auto& i : loaded_signatures())
	{
		if (i.name == name)
		{
			*out = i.address;
			return;
		}
	}

	*out = vtable::getVTableEx(name);

	assert(*out && "signature not found");
}

void signatures::init()
{
	addSignatures();

#ifdef _DEBUG
	if (failed)
		throw std::runtime_error("Failed to find signatures");
#endif
}