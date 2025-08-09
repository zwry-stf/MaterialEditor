#include "hooks.h"

#include "signatures.h"
#include "vtable.h"
#include <stdexcept>
#include <string>

#include <util/mem.h>
#include "interfaces.h"
#include <Windows.h>
#include <Detours/detours.h>
#pragma comment(lib, "Detours/Detours.lib")

#include <util/log.h>


// will be filled before init automatically
struct HookToInit
{
	HookToInit(void** detour, void* address, const std::string& name)
		: detour(detour), address(address), name(name) {}

	void** detour;
	void* original = nullptr;
	void* address = nullptr;
	std::string name;
};

struct VTableHookToInit : public HookToInit
{
	VTableHookToInit(void** detour, const std::string& name, const std::string& module, const std::string& vtable_name, int index)
		: HookToInit(detour, nullptr, name), module(module), vtable_name(vtable_name), index(index) {}

	std::string vtable_name;
	std::string module;
	int index;
	void* pSourceData = nullptr;
};

inline std::vector<HookToInit>*       hooks_to_init;
inline std::vector<VTableHookToInit>* vtable_hooks_to_init;


#define hook(orig, detour) { if (!DetourAttach( &(void*&)orig, (void*)detour )) {\
		return false; } }

static bool attachVtableHook(VTableHookToInit& hook)
{
	void** vtable = (void**)vtable::find(hook.module, hook.vtable_name);
	if (!vtable)
	{
		log::printError("failed to find vtable '%s' for hook '%s'", hook.vtable_name.c_str(), hook.name.c_str());
		return false;
	}

	log::println("found vtable '%s' at %p", hook.name.c_str(), vtable);

	hook.pSourceData = &vtable[hook.index];
	hook.original = *(void**)hook.pSourceData;
	if (!hook.original)
	{
		log::printError("failed to find function '%s' in vtable at %p", hook.name.c_str(), hook.pSourceData);
		return false;
	}

	DWORD oldProtect = 0;
	if (!VirtualProtect(hook.pSourceData, sizeof(void*), PAGE_READWRITE, &oldProtect))
	{
		log::printError("failed to change mem protection for hook '%s' at %p", hook.name.c_str(), hook.pSourceData);
		return false;
	}
	*(void**)hook.pSourceData = hook.detour;

	if (!VirtualProtect(hook.pSourceData, sizeof(void*), oldProtect, nullptr))
	{
		log::printWarning("failed to change back protection for hook '%s' at %p", hook.name.c_str(), hook.pSourceData);
	}

	return true;
}

void hooks::init()
{
	addHooks();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	// init hooks
	for (auto& hook : *hooks_to_init)
	{
		if (!hook.address)
			hook.address = signatures::get<void*>(hook.name);

		hook.original = hook.address;
		auto res = DetourAttach(&(void*&)hook.original, (void*)hook.detour);
		if (res != NO_ERROR)
		{
			log::printError("failed to create hook '%s' at %p, error: %d", hook.name.c_str(), hook.address, res);
			throw std::runtime_error("failed to create hooks");
		}

		log::println("hook attached '%s' at %p", hook.name.c_str(), hook.address);
	}

	for (auto& hook : *vtable_hooks_to_init)
	{
		if (!attachVtableHook(hook))
		{
			throw std::runtime_error("failed to create hooks");
		}

		log::println("hook attached '%s' at %p", hook.name.c_str(), hook.pSourceData);
	}

	DetourTransactionCommit();
}

void hooks::destroy()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	for (auto& hook : *hooks_to_init)
	{
		DetourDetach(&(void*&)hook.original, hook.detour);
	}

	for (auto& hook : *vtable_hooks_to_init)
	{
		assert(hook.pSourceData != nullptr);
		DWORD oldProtect = 0;
		VirtualProtect(hook.pSourceData, sizeof(void*), PAGE_READWRITE, &oldProtect);
		*(void**)(hook.pSourceData) = hook.original;
		VirtualProtect(hook.pSourceData, sizeof(void*), oldProtect, nullptr);
	}

	DetourTransactionCommit();
}

void hooks::registerHookEx(void** detour, void* func, const std::string& name)
{
	// order of global object initialization is not consistent
	static std::vector<HookToInit> _vector;
	hooks_to_init = &_vector;
	hooks_to_init->emplace_back(detour, func, name);
}

void hooks::registerHookVTableEx(void** detour, const std::string& name, const std::string& module, const std::string& vtable_name, int index)
{
	static std::vector<VTableHookToInit> _vector;
	vtable_hooks_to_init = &_vector;
	vtable_hooks_to_init->emplace_back(detour, name, module, vtable_name, index);
}

void* hooks::getOriginalEx(const std::string& name)
{
	for (auto& hook : *hooks_to_init)
	{
		if (hook.name == name)
			return hook.original;
	}

	for (auto& hook : *vtable_hooks_to_init)
	{
		if (hook.name == name)
			return hook.original;
	}

	assert(false && "failed to find hook");
}