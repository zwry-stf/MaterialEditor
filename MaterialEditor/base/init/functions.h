#pragma once
#include <stdint.h>
#include <string>


class functions
{
public:
	static void init();

public:
	template <typename T>
	inline static T get(const std::string& name)
	{
		T ret;
		getFunctionEx(name, (void**)&ret);
		return ret;
	}

private:
	template <typename T, typename = std::enable_if_t<
		std::is_pointer_v<T>>>
		static void addFunction(const std::string& name, T func)
	{
		addFunctionEx(name, (void*)func);
	}

	static void addFunctionEx(const std::string& name, void* address);
	static void getFunctionEx(const std::string& name, void** out);
};