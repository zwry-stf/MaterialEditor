#pragma once
#include <init/signatures.h>
#include <util/vec3.h>
#include <util/mem.h>
#include <init/schema.h>

class CSchemaClassInfoData;

enum class ELifeState : uint8_t
{
	ALIVE = 0,
	DYING,
	DEAD,
	RESPAWNABLE,
	DISCARDBODY
};

class C_BaseEntity
{
public:
	Add_Offset(C_BaseEntity, m_iHealth, int32_t);
	Add_Offset(C_BaseEntity, m_iMaxHealth, int32_t);
	Add_Offset(C_BaseEntity, m_iTeamNum, int32_t);
	Add_Offset(C_BaseEntity, m_fFlags, int32_t);
	Add_Offset(C_BaseEntity, m_vecVelocity, Vec3);
	Add_Offset(C_BaseEntity, m_vecAbsVelocity, Vec3);
	Add_Offset(C_BaseEntity, m_nSubclassID, uint64_t);
	Add_Offset(C_BaseEntity, m_nSimulationTick, int32_t);
	Add_Offset(C_BaseEntity, m_flSimulationTime, float);
	Add_Offset(C_BaseEntity, m_flWaterLevel, float);
	Add_Offset(C_BaseEntity, m_lifeState, ELifeState);

	CSchemaClassInfoData* getSchemaClassInfo() const
	{
		CSchemaClassInfoData* ret = nullptr;
		mem::CallVFunc<void, 42U>(this, &ret);

		return ret;
	}
};