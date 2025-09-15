#include "signatures.h"

#include "interfaces.h"

#include <stdexcept>

#ifdef _DEBUG
#include <Windows.h>
#endif


#define s SignatureString

void signatures::addSignatures()
{
#ifdef _DEBUG
	try
	{
		void* address;

		addSig("RandomPatch", "gameoverlayrenderer64.dll", s("48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 89 7C 24 20 55 48 8D AC 24 ? ? ? FF 48"), address);


		DWORD oldProtect = 0;
		if (!VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
			throw std::runtime_error("Failed to patch byte");

		*(uint8_t*)(address) = 0xC3; // return
		VirtualProtect(address, 1, oldProtect, &oldProtect);
	}
	catch (const std::runtime_error& e)
	{
		// Ignore
		((void)e);
		failed = false;
	}
#endif

	// render
	addSig("Swapchain", "GameOverlayRenderer64.dll", s("48 8B 05 {} 33 C9 48 89 48 18 48 8B 05"));

	// CResourceSystem
	addSig("CreateMaterial", "materialsystem2.dll", s("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05"));
	addSig("FixupResourceClient", "client.dll", s("E8 {} 48 8B D3 C7 85"));

	// CKeyValues3
	addSig("SetTypeKV3", "client.dll", s("E8 {} 48 8B 5E ? 41 8B 06"));

	// CGameEntitySystem
	addSig("GetEntityByIndex", "client.dll", s("4C 8D 49 ? 81 FA"));
}