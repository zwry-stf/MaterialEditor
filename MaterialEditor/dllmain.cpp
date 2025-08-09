#include "base/base.h"

#include <Windows.h>
#include <stdexcept>
#include <string>
#include <thread>


void error(std::string format, ...)
{
	va_list args;
	va_start(args, format);

	format = "failed to init: " + format;

	char buffer[512];
	vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format.c_str(), args);
	va_end(args);

	MessageBoxA(NULL, buffer, "Material editor | Error", MB_ICONERROR | MB_OK);
}

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
		error("%s", e.what());

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
		{
			error("failed to create thread");

			abort();
		}
		t.detach();
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		if (reserved == NULL) // Unloading
			base::destroy();
	}

	return TRUE;
}

