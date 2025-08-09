#pragma once
#include <string>
#include <assert.h>
#include <stdexcept>


class vtable
{
public:
	static void init();

	static void* find(const std::string& module, const std::string& vtable);
	static void* find(const std::string& module, const std::string& vtable, int index);

	static void** get(const std::string& name)
	{
		void* ret = getVTableEx(name);
		assert(ret);

		return (void**)ret;
	}

private:
	static void* getVTableEx(const std::string& name);

	static void add(const std::string& name, const std::string& module, const std::string& vtable, int i);
	static void addVTable(const std::string& name, const std::string& module, const std::string& vtable);

	friend class signatures;

private:
	static void addVTables();

private:
	inline static bool failed = false;
};