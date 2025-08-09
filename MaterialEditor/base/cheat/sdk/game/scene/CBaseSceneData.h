#pragma once
#include "../entity/CHandle.h"
#include "../util/CMaterial2.h"


class Color_t 
{
public:
	uint8_t r, g, b, a;

public:
	Color_t()
		: r(0), g(0), b(0), a(255) { }
	Color_t(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
		: r(red), g(green), b(blue), a(alpha) { }
	Color_t(const float* col)
		: r((uint8_t)(col[0] * 255.f)), g((uint8_t)(col[1] * 255.f)), b((uint8_t)(col[2] * 255.f)), a((uint8_t)(col[3] * 255.f)) { }
	Color_t(const Color_t& col)
		: r(col.r), g(col.g), b(col.b), a(col.a) { }
};

struct CSceneSkyboxObject
{
	SDK_CLASS(CSceneSkyboxObject);
public:
	MEM_PAD(0x110);
	CMaterial2** pSkyMaterial; // 0x110
	MEM_PAD(0x10);
	Color_t colorSky;
};

class CSkeletonInstance;
class CMeshInstance_Imp;
class CSceneAnimatableObjectDesc;
class C_BaseEntity;
class CSceneAnimatableObject
{
	SDK_CLASS(CSceneAnimatableObject);
public:
	MEM_PAD(0x10);
	CMeshInstance_Imp* m_pMeshInstance{ };
	CSceneAnimatableObjectDesc* m_desc{ };
	MEM_PAD(0x40);
	MEM_PAD(0x58);
	CHandle<C_BaseEntity> hOwner; // 0xB8
	MEM_PAD(0x4);
	MEM_PAD(0x50);
	CSkeletonInstance* m_pSkeletonInstance; // 0x110
	int m_nUnk1;
	int m_nUnk2;
	MEM_PAD(0x18);
	int m_nCount; // 0x138, 0x120
	MEM_PAD(0x4);
	void* m_pData; // 0x140
	MEM_PAD(0x10);
};

enum MaterialVarFlags_t
{
	MATERIAL_VAR_DEBUG					  = (1 << 0),
	MATERIAL_VAR_NO_DEBUG_OVERRIDE		  = (1 << 1),
	MATERIAL_VAR_NO_DRAW				  = (1 << 2),
	MATERIAL_VAR_USE_IN_FILLRATE_MODE	  = (1 << 3),

	MATERIAL_VAR_VERTEXCOLOR			  = (1 << 4),
	MATERIAL_VAR_VERTEXALPHA			  = (1 << 5),
	MATERIAL_VAR_SELFILLUM				  = (1 << 6),
	MATERIAL_VAR_ADDITIVE				  = (1 << 7),
	MATERIAL_VAR_ALPHATEST				  = (1 << 8),
	MATERIAL_VAR_ZNEARER				  = (1 << 10),
	MATERIAL_VAR_MODEL					  = (1 << 11),
	MATERIAL_VAR_FLAT					  = (1 << 12),
	MATERIAL_VAR_NOCULL					  = (1 << 13),
	MATERIAL_VAR_NOFOG					  = (1 << 14),
	MATERIAL_VAR_IGNOREZ				  = (1 << 15),
	MATERIAL_VAR_DECAL					  = (1 << 16),
	MATERIAL_VAR_ENVMAPSPHERE			  = (1 << 17), // OBSOLETE
//	MATERIAL_VAR_UNUSED					  = (1 << 18),
	MATERIAL_VAR_ENVMAPCAMERASPACE	      = (1 << 19), // OBSOLETE
	MATERIAL_VAR_BASEALPHAENVMAPMASK	  = (1 << 20),
	MATERIAL_VAR_TRANSLUCENT              = (1 << 21),
	MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
	MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING  = (1 << 23), // OBSOLETE
	MATERIAL_VAR_OPAQUETEXTURE			  = (1 << 24),
	MATERIAL_VAR_MULTIPLY				  = (1 << 25),
	MATERIAL_VAR_SUPPRESS_DECALS		  = (1 << 26),
	MATERIAL_VAR_HALFLAMBERT			  = (1 << 27),
	MATERIAL_VAR_WIREFRAME                = (1 << 28),
	MATERIAL_VAR_ALLOWALPHATOCOVERAGE     = (1 << 29),
	MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY  = (1 << 30),
	MATERIAL_VAR_VERTEXFOG				  = (1 << 31),
};

class CMeshDrawPrimitive_t
{
	SDK_CLASS(CMeshDrawPrimitive_t);
public:
	void* pData; // 0x0
	MEM_PAD(0x10); // 0x8
	CSceneAnimatableObject* pSceneAnimatableObject; // 0x18
	CMaterial2* pMaterial; // 0x20
	CMaterial2* pMaterial2; // 0x28
	MEM_PAD(0x10); // 0x30
	void* some_data;
	MEM_PAD(0x8);
	Color_t colValue; // 0x50
	MEM_PAD(0x14); // 0x54
}; //Size: 0x0068