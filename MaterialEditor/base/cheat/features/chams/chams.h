#pragma once
#include <imgui/imgui.h>
#include <cheat/sdk/game/util/CMaterial2.h>
#include <cheat/sdk/game/util/CTextureDx11.h>
#include <vector>
#include <mutex>
#include <string>


struct LoadedMaterial
{
	CStrongHandle<CMaterial2> mat;
	std::vector<CStrongHandle<CTextureDx11>> loaded_textures;
};


class CSceneAnimatableObjectDesc;
class CSceneAnimatableObject;
class CMeshDrawPrimitive_t;
namespace features
{
	class chams
	{
	public:
		inline static bool    players_only    = false;
		inline static bool    visible         = true;
		inline static ImColor visible_color   = { 1.f, 1.f, 1.f, 1.f };
		inline static bool    invisible       = false;
		inline static ImColor invisible_color = { 1.f, 1.f, 1.f, 1.f };

		static void init();
		static void destroy();

		static bool reloadVisible(const std::string& mat_name, const std::string& mat_contents);
		static bool reloadInvisible(const std::string& mat_name, const std::string& mat_contents);

		inline static std::string default_material_visible;
		inline static std::string default_material_invisible;

		static bool __fastcall hkDrawArray(CSceneAnimatableObjectDesc*,
			void*, CMeshDrawPrimitive_t*, int, void*, void*, void*);

		inline static std::vector<std::string> getAllocatedVisible()
		{
			std::vector<std::string> ret;

			std::lock_guard<std::mutex> lock(m_Mutex);
			for (auto& binding : m_CurrentMaterialVisible.loaded_textures)
				if (binding.pBinding && 
					binding.pBinding->szName &&
					*binding.pBinding->szName)
					ret.emplace_back(*binding.pBinding->szName);

			return std::move(ret);
		}

		inline static std::vector<std::string> getAllocatedInvisible()
		{
			std::vector<std::string> ret;

			std::lock_guard<std::mutex> lock(m_Mutex);
			for (auto& binding : m_CurrentMaterialInvisible.loaded_textures)
				if (binding.pBinding &&
					binding.pBinding->szName &&
					*binding.pBinding->szName)
					ret.emplace_back(*binding.pBinding->szName);

			return std::move(ret);
		}

	private:
		static void onDrawArray(CSceneAnimatableObjectDesc*,
			void*, CMeshDrawPrimitive_t*, int, void*, void*, void*);

	private:
		static void resetVisible();
		static void resetInvisible();

	private:
		inline static std::mutex m_Mutex;
		inline static LoadedMaterial m_CurrentMaterialVisible;
		inline static LoadedMaterial m_CurrentMaterialInvisible;
		inline static CStrongHandle<CMaterial2> m_DefaultMaterialVisible;
		inline static CStrongHandle<CMaterial2> m_DefaultMaterialInvisible;
	};
}