#include "vtable.h"

#include <Windows.h>
#include <stdexcept>
#include <assert.h>
#include <vector>
#include <fstream>
#include "signatures.h"
#include <sstream>
#include <util/log.h>


struct LoadedVTable
{
	std::string name;
	void* address;
};
inline static std::vector<LoadedVTable> loaded_vtables;


std::vector<int16_t> bytes_to_ida_pattern(byte* bytes, int32_t size)
{
	std::vector<int16_t> ret;
	for (int i = 0; i < size; i++)
		ret.push_back(bytes[i]);

	return ret;
}

std::vector<uint64_t> get_xrefs_to(uint64_t address, uint64_t start, uint64_t size)
{
	std::vector<uint64_t> xrefs = {};

	// Convert the address over to an IDA pattern string
	const std::vector<int16_t> ida_pattern = bytes_to_ida_pattern((byte*)&address, 4);

	// Get the end of the section (in our case the end of the .rdata section)
	const uint64_t end = start + size;
	while (start && start < end)
	{
		uint64_t xref = (uint64_t)signatures::patternScan((void*)start, (void*)(start + size), ida_pattern, 0, 0);

		// If the xref is 0 it means that there either were no xrefs, or there are no remaining xrefs.
		// So we should break out of our loop, otherwise it will keep on trying to look for xrefs.
		if (!xref)
			break;

		// We've found an xref, save it in the vector, and add 4 to start, so it wil now search for xrefs
		// from the previously found xref untill we're at the end of the section, or there aren't any xrefs left.
		xrefs.push_back(xref);
		start = xref + 4;
	}

	return xrefs;
}

bool get_section_info(uint64_t base_address, const std::string& section_name, uint64_t& section_start, uint64_t& section_size)
{
	const PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)base_address;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	const PIMAGE_NT_HEADERS32 nt_headers = (PIMAGE_NT_HEADERS32)(base_address + dos_header->e_lfanew);
	if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
		return false;

	PIMAGE_SECTION_HEADER section_header = IMAGE_FIRST_SECTION(nt_headers);
	uint16_t number_of_sections = nt_headers->FileHeader.NumberOfSections;
	while (number_of_sections > 0)
	{
		// If we're at the right section
		if (!strcmp(section_name.c_str(), (const char*)section_header->Name))
		{
			section_start = base_address + section_header->VirtualAddress;
			section_size = section_header->SizeOfRawData;
			return true;
		}

		section_header++;
		number_of_sections--;
	}

	return false;
}

void* get_vtable_pointer(uint64_t base_address, size_t base_size, const std::string& table_name)
{
	// Type descriptor names look like this: .?AVC_CSPlayer@@ (so: ".?AV" + table_name + "@@")
	const std::string type_descriptor_name = ".?AV" + table_name + "@@";

	// Convert the string to an IDA pattern so that we can pattern scan it
	std::vector<int16_t> ida_pattern = bytes_to_ida_pattern((byte*)type_descriptor_name.data(), (int32_t)type_descriptor_name.size());

	uint64_t rtti_type_descriptor = (uint64_t)signatures::patternScan((void*)base_address, (void*)(base_address + base_size), ida_pattern, 0, 0);
	if (!rtti_type_descriptor)
		return nullptr;

	// We're doing - 0x8 here, because the location of the rtti_type_descriptor is 0x8 bytes before the string (open up client_panorama.dll in IDA and take a look) 
	rtti_type_descriptor -= 0x10;

	// We only need to get xrefs that are inside the .rdata section (there sometimes are xrefs in .text, so we have to filter them out)
	uint64_t rdata_start = 0, rdata_size = 0;
	if (!get_section_info(base_address, ".rdata", rdata_start, rdata_size))
		return nullptr;

	// Get all xrefs to the type_descriptor
	const std::vector<uint64_t> xrefs = get_xrefs_to(rtti_type_descriptor - base_address, rdata_start, rdata_size);
	for (const uint64_t xref : xrefs)
	{
		// xref - 0x8 = offset of this vtable in complete class (from top)
		// So if it's 0 it means it's the class we need, and not some class it inherits from (again, opening up client_panorama.dll in IDA will help you understand)
		const int32_t offset_from_class = *(int32_t*)(xref - 0x8);
		if (offset_from_class != 0)
			continue;

		// We've found the correct xref, the object locator is 0xC bytes before the xref. (Again, client_panorama.dll yada yada yada)
		uint64_t object_locator = xref - 0xC;
		//uint64_t mod = object_locator % 8;
		//if (mod != 0)
		//	object_locator += 8 - mod;

		// Now we need to get an xref to the object locator, as that's where the vtable is located
		{
			// Convert the object locator address to an IDA pattern
			ida_pattern = bytes_to_ida_pattern((byte*)&object_locator, sizeof(int));

			uint64_t vtable_address = (uint64_t)signatures::patternScan((void*)rdata_start, (void*)(rdata_start + rdata_size), ida_pattern, 0, 0);

			// Here I'm checking for <= 4 as we're adding 0x4 to it. So if the pattern scan returns 0 we still head the [removed] out
			if (vtable_address == 0)
				continue;

			vtable_address += sizeof(uint64_t);

			return (void*)vtable_address;
		}
	}

	// We for some odd reason didn't find any valid xrefs
	return nullptr;
}

void* vtable::find(const std::string& module, const std::string& vtable)
{
	uint64_t mod = (uint64_t)GetModuleHandleA(module.c_str());
	if (!mod)
		return nullptr;

	auto DosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(mod);
	auto NTHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(mod + DosHeader->e_lfanew);
	DWORD size = NTHeaders->OptionalHeader.SizeOfImage;

	return get_vtable_pointer((uint64_t)mod, static_cast<size_t>(size), vtable);
}

void* vtable::find(const std::string& module, const std::string& vtable, int index)
{
	void** res = (void**)find(module, vtable);
	if (!res)
		return nullptr;

	return res[index];
}

void* vtable::getVTableEx(const std::string& name)
{
	for (auto& i : loaded_vtables)
		if (i.name == name)
			return i.address;

	return nullptr;
}

void vtable::add(const std::string& name, const std::string& module, const std::string& vtable, int i)
{
	void* address = find(module, vtable, i);

	if (!address)
	{
		log::printError("failed to find vtable '%s'", name.c_str());
		throw std::runtime_error("failed to find vtables");
	}

	log::println("vtable function '%s' found at %p", name.c_str(), address);

	loaded_vtables.emplace_back(name, address);
}

void vtable::addVTable(const std::string& name, const std::string& module, const std::string& vtable)
{
	void* address = find(module, vtable);

	if (!address)
	{
		log::printError("failed to find vtable '%s'", vtable.c_str());
		throw std::runtime_error("failed to find vtables");
	}

	log::println("vtable '%s' found at %p", name.c_str(), address);

	loaded_vtables.emplace_back(name, address);
}

void vtable::init()
{
	addVTables();

#ifdef _DEBUG
	if (failed)
		throw std::runtime_error("Failed to find vtables");
#endif
}