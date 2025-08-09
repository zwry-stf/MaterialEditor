#pragma once
#include <string>
#include <type_traits>


class hooks
{
public:
	static void init();
	static void destroy();


	template<typename Fn, typename A,
		typename = std::enable_if_t<
		std::is_pointer_v<A>&&
		std::is_function_v<std::remove_pointer_t<Fn>>
		>>
		static void registerHook(Fn& detour, A target, const std::string& name)
	{
		registerHookEx(reinterpret_cast<void**>(&detour),
			reinterpret_cast<void*>(target), name);
	}

	template<typename Fn,
		typename = std::enable_if_t<
		std::is_function_v<std::remove_pointer_t<Fn>>
		>>
		static void registerHook(Fn& detour, const std::string& name)
	{
		registerHookEx(reinterpret_cast<void**>(&detour),
			nullptr, name);
	}

	template<typename Fn,
		typename = std::enable_if_t<
		std::is_function_v<std::remove_pointer_t<Fn>>
		>>
		static void registerHookVTable(Fn& detour, const std::string& name, const std::string& module, const std::string& vtable_name, int index)
	{
		registerHookVTableEx(reinterpret_cast<void**>(&detour),
			name, module, vtable_name, index);
	}

	template <typename T>
	inline static T getOriginal(const std::string& name)
	{
		return (T)getOriginalEx(name);
	}

private:
	static void registerHookEx(void** detour, void* func, const std::string& name);
	static void registerHookVTableEx(void** detour, const std::string& name, const std::string& module, const std::string& vtable_name, int index);
	static void* getOriginalEx(const std::string& name);

	static void addHooks();
};


struct global_hook_loader
{
	template<typename Fn,
		typename = std::enable_if_t<
		std::is_function_v<std::remove_pointer_t<Fn>>
		>>
		global_hook_loader(Fn& detour, const std::string& name)
	{
		::hooks::registerHook(detour, name);
	}
};

#define RegisterHookGlobal(detour, name) namespace \
{ \
    inline static global_hook_loader _reg_##name = global_hook_loader(detour, #name); \
}