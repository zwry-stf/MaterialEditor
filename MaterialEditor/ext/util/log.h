#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <assert.h>


#define zyra_logging_enabled

class log
{
public:
	static void print(std::string format, ...)
	{
#ifdef zyra_logging_enabled
		va_list args;
		va_start(args, format);

		assert(!format.empty());

		char buffer[512];
		vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format.c_str(), args);
		va_end(args);

		OutputDebugStringA(buffer);
#endif // zyra_logging_enabled
	}

	template <class... types>
	static void println(const std::string& format, types&&... _Args)
	{
		print(format + "\n", std::forward<types>(_Args)...);
	}

	template <class... types>
	static void printWarning(const std::string& format, types&&... _Args)
	{
		println("Warning: " + format, std::forward<types>(_Args)...);
	}

	template <class... types>
	static void printError(const std::string& format, types&&... _Args)
	{
		println("Error: " + format, std::forward<types>(_Args)...);
	}
};