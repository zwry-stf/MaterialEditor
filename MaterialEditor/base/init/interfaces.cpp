#include "interfaces.h"
#include <cheat/sdk/game/util/module.h>
#include <cheat/sdk/interfaces/CResourceSystem.h>

#include <stdexcept>
#include <util/log.h>


static void interface_error(const char* name)
{
    log::printError("failed to find interface '%s'", name);
    throw std::runtime_error("failed to find interfaces");
}

void interfaces::init()
{
    addGameInterface("schemasystem.dll", "SchemaSystem_00", iSchemaSystem);
    addGameInterface("inputsystem.dll", "InputSystemVersion00", iInputSystem);
    addGameInterface("resourcesystem.dll", "ResourceSystem013", iResourceSystem);
    addGameInterface("engine2.dll", "GameResourceServiceClientV00", iGameResourceService);

    // resourcesystem
    iResourceHandleUtils = (CResourceHandleUtils*)iResourceSystem->QueryInterface("ResourceHandleUtils001");
    if (!iResourceHandleUtils)
        interface_error("IResourceHandleUtils");
}

void interfaces::addGameInterfaceEx(const char* module, const char* name, void** out)
{
    CInterfaceRegister* Register = module::getRegister(module);
    if (!Register)
    {
        interface_error(name);
    }

    *out = module::capture<void>(Register, name);
    if (*out == nullptr)
    {
        interface_error(name);
    }

    log::println("interface '%s' found at %p", name, *out);
}