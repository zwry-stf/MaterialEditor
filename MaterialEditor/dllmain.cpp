#include "base/base.h"

#include <Windows.h>
#include <stdexcept>
#include <string>
#include <thread>


static void init()
{
#ifdef NDEBUG
	try
	{
#endif
		base::init();
#ifdef NDEBUG
	}
	catch (const std::runtime_error& e)
	{
		MessageBoxA(NULL,
			(std::string(_("failed to init material editor: ")) + e.what()).c_str(),
			_("Material editor | Error"), MB_ICONERROR | MB_OK);

		abort();
	}
#endif
}


int APIENTRY DllMain(HMODULE, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		std::thread t(init);
		if (!t.joinable())
			MessageBoxA(NULL,
				"failed to create thread",
				"Material editor | Error", MB_ICONERROR | MB_OK);
		t.detach();
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
#ifdef _DEBUG
		if (reserved == NULL) // Unloading
			base::destroy();
#else
		return FALSE;
#endif
	}

	return TRUE;
}

