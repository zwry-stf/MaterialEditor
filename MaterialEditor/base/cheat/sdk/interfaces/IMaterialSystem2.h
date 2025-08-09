#include <cheat/sdk/game/util/CMaterial2.h>


class IMaterialSystem2
{
	SDK_CLASS(IMaterialSystem2);
public:
	CMaterial2*** findOrCreateFromResource(CMaterial2*** pOutMaterial, const char* szMaterialName)
	{
		return mem::CallVFunc<CMaterial2***, 14U>(this, pOutMaterial, szMaterialName);
	}
};