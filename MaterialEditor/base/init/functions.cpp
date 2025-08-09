#include "functions.h"
#include "interfaces.h"
#include <render/render.h>
#include <cheat/sdk/game/util/module.h>
#include <init/hooks.h>
#include <util/mem.h>
#include <stdexcept>
#include <vector>
#include <util/log.h>


struct LoadedFunction
{
	std::string name;
	void* address;
};

inline static std::vector<LoadedFunction> loaded_functions;

#define cast(x, y) x = reinterpret_cast<decltype(x)>(y)

void functions::init()
{
	// -- Hooks --
	// render
	hooks::registerHook(render::hkSetRelativeMouseMode,
		mem::GetVFunc(interfaces::iInputSystem, 76),
		"SetRelativeMouseMode");

	// -- Functions --
	// CKeyValues3
	addFunction("LoadKeyValues",
		module::getSigExportAddress("tier0.dll", "?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEAVCUtlBuffer@@AEBUKV3ID_t@@PEBDI@Z"));

	// CUtlBuffer
	addFunction("CUtlBuffer::Init",
		module::getSigExportAddress("tier0.dll", "??0CUtlBuffer@@QEAA@HHW4BufferFlags_t@0@@Z"));

	addFunction("CUtlBuffer::Init2",
		module::getSigExportAddress("tier0.dll", "??0CUtlBuffer@@QEAA@PEBXHW4BufferFlags_t@0@@Z"));

	addFunction("CUtlBuffer::PutString",
		module::getSigExportAddress("tier0.dll", "?PutString@CUtlBuffer@@QEAAXPEBD@Z"));

	addFunction("CUtlBuffer::EnsureCapacity",
		module::getSigExportAddress("tier0.dll", "?EnsureCapacity@CUtlBuffer@@QEAAXH@Z"));

	// CBufferString
	addFunction("CBufferString::Insert",
		module::getSigExportAddress("tier0.dll", "?Insert@CBufferString@@QEAAPEBDHPEBDH_N@Z"));

	addFunction("CBufferString::Purge",
		module::getSigExportAddress("tier0.dll", "?Purge@CBufferString@@QEAAXH@Z"));

	addFunction("CBufferString::SetExtension",
		module::getSigExportAddress("tier0.dll", "?SetExtension@CBufferString@@QEAAPEBDPEBD_N@Z"));
}

void functions::addFunctionEx(const std::string& name, void* address)
{
	if (!address)
	{
		log::printError("failed to find function '%s'", name.c_str());
		throw std::runtime_error("failed to get functions");
	}

	log::println("function '%s' found at %p", name.c_str(), address);

	loaded_functions.emplace_back(name, address);
}

void functions::getFunctionEx(const std::string& name, void** out)
{
	for (auto& i : loaded_functions)
	{
		if (i.name == name)
		{
			*out = i.address;
			return;
		}
	}

	assert(false && "failed to find function");
}