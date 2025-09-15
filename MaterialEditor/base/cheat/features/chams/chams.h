#pragma once
#include <imgui/imgui.h>
#include <cheat/sdk/game/util/CMaterial2.h>
#include <cheat/sdk/game/util/CTextureDx11.h>
#include <vector>
#include <mutex>
#include <string>
#include <atomic>


struct LoadedTexture
{
	CStrongHandle<CTextureDx11> tex;
	bool need_destroy = false;
};

struct LoadedMaterial
{
	CStrongHandle<CMaterial2> mat;
	std::vector<LoadedTexture> loaded_textures;
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

		struct Error
		{
			std::atomic<bool> pending = false;
			std::string message;
		};
		inline static Error error;

		static void reloadVisible(const std::string& mat_name, const std::string& mat_contents);
		static void reloadInvisible(const std::string& mat_name, const std::string& mat_contents);

		inline static std::string default_material_visible;
		inline static std::string default_material_invisible;

		static void __fastcall hkFrameStageNotify(void*, int);
		static bool __fastcall hkDrawArray(CSceneAnimatableObjectDesc*,
			void*, CMeshDrawPrimitive_t*, int, void*, void*, void*);

		inline static std::vector<std::string> getAllocatedVisible()
		{
			std::vector<std::string> ret;

			std::lock_guard<std::mutex> lock(m_Mutex);
			for (auto& texture : m_CurrentMaterialVisible.loaded_textures)
				if (texture.tex.pBinding &&
					texture.tex.pBinding->szName &&
					*texture.tex.pBinding->szName)
					ret.emplace_back(*texture.tex.pBinding->szName);

			return std::move(ret);
		}

		inline static std::vector<std::string> getAllocatedInvisible()
		{
			std::vector<std::string> ret;

			std::lock_guard<std::mutex> lock(m_Mutex);
			for (auto& texture : m_CurrentMaterialInvisible.loaded_textures)
				if (texture.tex.pBinding &&
					texture.tex.pBinding->szName &&
					*texture.tex.pBinding->szName)
					ret.emplace_back(*texture.tex.pBinding->szName);

			return std::move(ret);
		}

	private:
		static void onDrawArray(CSceneAnimatableObjectDesc*,
			void*, CMeshDrawPrimitive_t*, int, void*, void*, void*);

	private:
		static void resetVisible();
		static void resetInvisible();

		static bool reloadVisible();
		static bool reloadInvisible();

	private:
		inline static std::atomic<bool> m_PendingReloadVisible   = false;
		inline static std::atomic<bool> m_PendingReloadInvisible = false;
		inline static std::string m_PendingMatVisible;
		inline static std::string m_PendingMatSourceVisible;
		inline static std::string m_PendingMatInvisible;
		inline static std::string m_PendingMatSourceInvisible;

	private:
		inline static std::mutex m_Mutex;
		inline static LoadedMaterial m_CurrentMaterialVisible;
		inline static LoadedMaterial m_CurrentMaterialInvisible;
		inline static CStrongHandle<CMaterial2> m_DefaultMaterialVisible;
		inline static CStrongHandle<CMaterial2> m_DefaultMaterialInvisible;

		inline static std::mutex m_DestroyMutex;
		inline static std::vector<CStrongHandle<CTextureDx11>> m_DestroyTextures;
	};
}