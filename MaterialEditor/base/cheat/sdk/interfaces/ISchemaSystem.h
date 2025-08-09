#pragma once
#include <util/mem.h>
#include <cheat/sdk/sdk_object.h>


class CSchemaClassInfoData;

class SchemaClassFieldData_t
{
	SDK_CLASS(SchemaClassFieldData_t);
public:
	const char* szName; // 0x0000
	void* pSchemaType; // 0x0008
	uint32_t nSingleInheritanceOffset; // 0x0010
	int32_t nMetadataSize; // 0x0014
	void* pMetaData; // 0x0018
};

class SchemaBaseClassInfoData_t
{
	SDK_CLASS(SchemaBaseClassInfoData_t);
public:
	int32_t nOffset;
	MEM_PAD(0x4);
	CSchemaClassInfoData* pClass;
};

class CSchemaSystemTypeScope;
class CSchemaDeclaredClass
{
	SDK_CLASS(CSchemaDeclaredClass);
public:
	void* vfptr;
	const char* m_szName;
	CSchemaSystemTypeScope* m_pCSchemaSystemTypeScope;
	const char* m_szUnknownStr;
	CSchemaClassInfoData* m_Class;
};

class CSchemaDeclaredClassEntry
{
	SDK_CLASS(CSchemaDeclaredClassEntry);
public:
	uint64_t m_nHash[2];
	CSchemaDeclaredClass* m_pDeclaredClass;
};

class CSchemaSystemTypeScope
{
	SDK_CLASS(CSchemaSystemTypeScope);
public:
	void FindDeclaredClass(CSchemaClassInfoData** pReturnClass, const char* szClassName)
	{
		return mem::CallVFunc<void, 2U>(this, pReturnClass, szClassName);
	}

	void* pVtable; // 0x0000
	char szName[256U]; // 0x0008
	MEM_PAD(0x338);
	uint16_t m_nNumDeclaredClasses;
	MEM_PAD(0x6);
	CSchemaDeclaredClassEntry* m_pDeclaredClasses;
};

class CSchemaClassInfoData
{
	SDK_CLASS(CSchemaClassInfoData);
private:
	void* pVtable; // 0x0000
public:
	const char* szName; // 0x0008
	char* szDescription; // 0x0010

	int m_nSize; // 0x0018
	int16_t nFieldSize; // 0x001C
	int16_t nStaticSize; // 0x001E
	int16_t nMetadataSize; // 0x0020

	MEM_PAD(0x2); // 0x0022
	uint8_t nBaseClassesCount; // 0x0024
	MEM_PAD(0x3); // 0x0025
	SchemaClassFieldData_t* pFields; // 0x0028
	SchemaBaseClassInfoData_t* pBaseClasses; // 0x0030
	MEM_PAD(0x38); // 0x0038

	bool InheritsFrom(CSchemaClassInfoData* pClassInfo)
	{
		if (pClassInfo == this && pClassInfo != nullptr)
			return true;
		else if (pBaseClasses == nullptr || pClassInfo == nullptr)
			return false;

		if (nBaseClassesCount > 0)
		{
			auto& baseClass = pBaseClasses[0];

			if (baseClass.pClass->InheritsFrom(pClassInfo))
				return true;
		}

		return false;
	}
};


class ISchemaSystem
{
	SDK_CLASS(ISchemaSystem);
public:
	CSchemaSystemTypeScope* FindTypeScopeForModule(const char* m_module_name)
	{
		return mem::CallVFunc<CSchemaSystemTypeScope*, 13U>(this, m_module_name, nullptr);
	}
};