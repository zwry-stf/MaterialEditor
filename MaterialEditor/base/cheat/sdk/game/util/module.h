#pragma once
#include <util/mem.h>
#include <Windows.h>
#include <string>


/// Interface
using InstantiateInterfaceFn_t = void* (*)();
class CInterfaceRegister
{
public:
    InstantiateInterfaceFn_t fnCreate;
    const char* szName;
    CInterfaceRegister* pNext;
};

class module
{
public:
    inline static CInterfaceRegister* getRegister(const char* name)
    {
        HMODULE _module = GetModuleHandleA(name);
        if (_module == 0)
            return nullptr;

        void* createInterface = GetProcAddress(_module, "CreateInterface");

        if (createInterface == 0)
            return nullptr;

        return *reinterpret_cast<CInterfaceRegister**>(mem::resolveRelativeAddress(createInterface, 0x3, 0x7));
    }

    // Find Interface in Register
    template <typename T = void*>
    inline static T* capture(const CInterfaceRegister* pModuleRegister, const char* szInterfaceName)
    {
        size_t len = strlen(szInterfaceName);
        for (const CInterfaceRegister* pRegister = pModuleRegister; pRegister != nullptr; pRegister = pRegister->pNext)
        {
            if (strncmp(pRegister->szName, szInterfaceName, len) == 0)
            {
                void* pInterface = pRegister->fnCreate();
                return static_cast<T*>(pInterface);
            }
        }
        return nullptr;
    }

    inline static void* getModuleSize(const HMODULE hModule)
    {
        auto DosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(hModule);
        auto NTHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(hModule + DosHeader->e_lfanew);
        return hModule + NTHeaders->OptionalHeader.SizeOfImage;
    }


    inline static void* getSigExportAddress(const char* module_name, const char* szProcedureName)
    {
        void* hModuleBase = reinterpret_cast<void*>(GetModuleHandle(module_name));

        const auto pBaseAddress = static_cast<const uint8_t*>(hModuleBase);

        const auto pIDH = static_cast<const IMAGE_DOS_HEADER*>(hModuleBase);
        if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
            return nullptr;

        const auto pINH = reinterpret_cast<const IMAGE_NT_HEADERS64*>(pBaseAddress + pIDH->e_lfanew);
        if (pINH->Signature != IMAGE_NT_SIGNATURE)
            return nullptr;

        const IMAGE_OPTIONAL_HEADER64* pIOH = &pINH->OptionalHeader;
        const uint64_t nExportDirectorySize = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
        const uint64_t uExportDirectoryAddress = pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

        if (nExportDirectorySize == 0U || uExportDirectoryAddress == 0U)
        {
            return nullptr;
        }

        const auto pIED = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(pBaseAddress + uExportDirectoryAddress);
        const auto pNamesRVA = reinterpret_cast<const uint32_t*>(pBaseAddress + pIED->AddressOfNames);
        const auto pNameOrdinalsRVA = reinterpret_cast<const uint16_t*>(pBaseAddress + pIED->AddressOfNameOrdinals);
        const auto pFunctionsRVA = reinterpret_cast<const uint32_t*>(pBaseAddress + pIED->AddressOfFunctions);

        // Perform binary search to find the export by name
        size_t nRight = pIED->NumberOfNames, nLeft = 0U;
        while (nRight != nLeft)
        {
            // Avoid INT_MAX/2 overflow
            const size_t uMiddle = nLeft + ((nRight - nLeft) >> 1U);
            if (reinterpret_cast<const char*>(pBaseAddress + pNamesRVA[uMiddle]) == nullptr)
                continue;

            const int iResult = strcmp(szProcedureName, reinterpret_cast<const char*>(pBaseAddress + pNamesRVA[uMiddle]));

            if (iResult == 0)
            {
                const uint32_t uFunctionRVA = pFunctionsRVA[pNameOrdinalsRVA[uMiddle]];

                // Check if it's a forwarded export
                if (uFunctionRVA >= uExportDirectoryAddress && uFunctionRVA - uExportDirectoryAddress < nExportDirectorySize)
                {
                    // Forwarded exports are not supported
                    break;
                }

                return const_cast<uint8_t*>(pBaseAddress) + uFunctionRVA;
            }

            if (iResult > 0)
                nLeft = uMiddle + 1;
            else
                nRight = uMiddle;
        }

        // Export not found
        return nullptr;
    }
};