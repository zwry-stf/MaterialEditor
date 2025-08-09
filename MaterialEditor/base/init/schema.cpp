#include "schema.h"
#include <cheat/sdk/interfaces/ISchemaSystem.h>
#include "interfaces.h"

#include <stdexcept>
#include <vector>
#include <assert.h>


uint32_t schema::getOffset(const std::string& path)
{
	std::string className, fieldName;

	size_t delimiterPos = path.find("->");

	if (delimiterPos != std::string::npos)
	{
		className = path.substr(0, delimiterPos); // Text before "->"
		fieldName = path.substr(delimiterPos + 2); // Text after "->"
	}
	else
	{
		throw std::runtime_error("invalid path: " + path);
	}


	CSchemaClassInfoData* pClass = nullptr;
	interfaces::iSchemaSystem->FindTypeScopeForModule("client.dll")->FindDeclaredClass(&pClass, className.c_str());

	if (!pClass)
	{
		throw std::runtime_error("invalid class: " + className);
	}

	for (int i = 0; i < pClass->nFieldSize; i++)
	{
		auto& field = pClass->pFields[i];

		if (strcmp(field.szName, fieldName.c_str()) == 0)
		{
			if (field.nSingleInheritanceOffset == 0)
			{
				throw std::runtime_error("invalid field offset for: " + fieldName);
			}
			return field.nSingleInheritanceOffset;
		}
	}

	throw std::runtime_error("failed to find schema: " + path);

	return 0;
}