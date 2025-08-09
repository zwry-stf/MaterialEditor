#pragma once
#include "../game/CGameEntitySystem.h"
#include <util/mem.h>


class CGameResourceService
{
	SDK_CLASS(CGameResourceService);
public:
	MEM_PAD(0x58);
	CGameEntitySystem* pGameEntitySystem;
};