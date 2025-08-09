#pragma once
class ISchemaSystem;
class CInputSystem;
class CResourceSystem;
class CResourceHandleUtils;
class CGameResourceService;


class interfaces
{
public:
	static void init();

private:
	template <typename T>
	inline static void addGameInterface(const char* module, const char* name, T& out)
	{
		addGameInterfaceEx(module, name, (void**)&out);
	}
	static void addGameInterfaceEx(const char* module, const char* name, void** out);

public:
	inline static ISchemaSystem*        iSchemaSystem        = nullptr;
	inline static CInputSystem*         iInputSystem         = nullptr;
	inline static CResourceSystem*      iResourceSystem      = nullptr;
	inline static CResourceHandleUtils* iResourceHandleUtils = nullptr;
	inline static CGameResourceService* iGameResourceService = nullptr;
};