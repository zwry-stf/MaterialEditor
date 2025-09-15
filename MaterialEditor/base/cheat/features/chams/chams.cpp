#include "chams.h"
#include <init/interfaces.h>
#include <cheat/sdk/interfaces/CResourceSystem.h>
#include <init/hooks.h>
#include <cheat/sdk/game/scene/CBaseSceneData.h>
#include <cheat/sdk/interfaces/ISchemaSystem.h>
#include <cheat/sdk/game/entity/C_BaseEntity.h>


const std::string key_values_sig = R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
)";

static CStrongHandle<CMaterial2> compile(const std::string& mat_name, const std::string& mat_contents)
{
    return CResourceSystem::createMaterial(
        mat_name.c_str(),
        (key_values_sig + mat_contents + "\n}").c_str()
    );
}

void features::chams::init()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    // create default materials
    // visible
    default_material_visible = 
R"(shader = "csgo_unlitgeneric.vfx"

F_PAINT_VERTEX_COLORS = 1
F_TRANSLUCENT = 1
F_BLEND_MODE = 1

g_vColorTint = [1, 1, 1, 1]

TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex")";

    m_DefaultMaterialVisible = compile("materials/dev/primary_white.vmat", default_material_visible);

    // invisible
    default_material_invisible = 
R"(shader = "csgo_unlitgeneric.vfx"

F_PAINT_VERTEX_COLORS = 1
F_TRANSLUCENT = 1
F_BLEND_MODE = 1
F_DISABLE_Z_BUFFERING = 1

g_vColorTint = [1, 1, 1, 1]

TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex")";

    m_DefaultMaterialInvisible = compile("materials/dev/primary_white.vmat", default_material_invisible);

    m_CurrentMaterialVisible.mat = m_DefaultMaterialVisible;
    m_CurrentMaterialInvisible.mat = m_DefaultMaterialInvisible;
}

void features::chams::destroy()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    // destroy default materials
    if (m_DefaultMaterialVisible.pBinding)
        interfaces::iResourceHandleUtils->deleteResource(m_DefaultMaterialVisible.pBinding);
    m_DefaultMaterialVisible.pBinding = nullptr;
    if (m_DefaultMaterialInvisible.pBinding)
        interfaces::iResourceHandleUtils->deleteResource(m_DefaultMaterialInvisible.pBinding);
    m_DefaultMaterialInvisible.pBinding = nullptr;

    resetVisible();
    resetInvisible();
}

#include <util/log.h>
void features::chams::reloadVisible(const std::string& mat_name, const std::string& mat_contents)
{
    m_PendingMatVisible = mat_name;
    m_PendingMatSourceVisible = mat_contents;
    m_PendingReloadVisible.store(true, std::memory_order_release);
}

void features::chams::reloadInvisible(const std::string& mat_name, const std::string& mat_contents)
{
    m_PendingMatInvisible = mat_name;
    m_PendingMatSourceInvisible = mat_contents;
    m_PendingReloadInvisible.store(true, std::memory_order_release);
}

bool features::chams::reloadVisible()
{
    auto ret = compile(m_PendingMatVisible, m_PendingMatSourceVisible);
    if (!ret.pBinding)
        return false;

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        resetVisible();
    }

    std::vector<LoadedTexture> textures;
    for (size_t pos = m_PendingMatSourceVisible.find("resource:\"");
        pos != std::string::npos; pos = m_PendingMatSourceVisible.find("resource:\"", pos + 10))
    {
        size_t start = pos + 10;
        size_t end = m_PendingMatSourceVisible.find(".vtex\"", start);
        if (end == std::string::npos)
            return false;

        end += 5;

        std::string path = m_PendingMatSourceVisible.substr(start, end - start);
        CBufferString buffer(path.c_str(), BuildExtension("vtex"));
        bool loaded = false;
        auto handle = interfaces::iResourceSystem->getResource(&buffer, 0LL);
        if (handle &&
            handle->pData)
            loaded = true;
        if (!loaded)
        {
            auto handle = interfaces::iResourceSystem->loadResource(&buffer);
            if (!handle)
                log::printWarning("failed to load texture '%s', ignoring...", path.c_str());
            else
            {
                textures.emplace_back(CStrongHandle<CTextureDx11>(), true);
                textures.back().tex.pBinding = handle;
            }
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_DestroyMutex);
            for (auto& tex : m_DestroyTextures)
                if (tex.pBinding == handle)
                {
                    textures.emplace_back(CStrongHandle<CTextureDx11>(), false);
                    textures.back().tex.pBinding = handle;
                }
        }
    }

    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CurrentMaterialVisible.loaded_textures = std::move(textures);
    m_CurrentMaterialVisible.mat = ret;

    return true;
}

bool features::chams::reloadInvisible()
{
    auto ret = compile(m_PendingMatInvisible, m_PendingMatSourceInvisible);
    if (!ret.pBinding)
        return false;

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        resetInvisible();
    }

    std::vector<LoadedTexture> textures;
    for (size_t pos = m_PendingMatSourceInvisible.find("resource:\"");
        pos != std::string::npos; pos = m_PendingMatSourceInvisible.find("resource:\"", pos + 9))
    {
        size_t start = pos + 10;
        size_t end = m_PendingMatSourceInvisible.find(".vtex\"", start);
        if (end == std::string::npos)
            return false;

        end += 5;

        std::string path = m_PendingMatSourceInvisible.substr(start, end - start);
        CBufferString buffer(path.c_str(), BuildExtension("vtex"));
        bool loaded = false;
        auto handle = interfaces::iResourceSystem->getResource(&buffer, 0LL);
        if (handle &&
            handle->pData)
            loaded = true;
        if (!loaded)
        {
            auto handle = interfaces::iResourceSystem->loadResource(&buffer);
            if (!handle)
                log::printWarning("failed to load texture '%s', ignoring...", path.c_str());
            else
            {
                textures.emplace_back(CStrongHandle<CTextureDx11>(), true);
                textures.back().tex.pBinding = handle;
            }
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_DestroyMutex);
            for (auto& tex : m_DestroyTextures)
                if (tex.pBinding == handle)
                {
                    textures.emplace_back(CStrongHandle<CTextureDx11>(), false);
                    textures.back().tex.pBinding = handle;
                }
        }
    }

    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CurrentMaterialInvisible.loaded_textures = std::move(textures);
    m_CurrentMaterialInvisible.mat = ret;

    return true;
}


void __fastcall features::chams::hkFrameStageNotify(void* client, int frame_stage)
{
    static auto oFrameStageNotify = hooks::getOriginal<
        void(__fastcall*)(void*, int)>
        ("FrameStageNotify");

    oFrameStageNotify(client, frame_stage);

    {
        std::lock_guard<std::mutex> lock(m_DestroyMutex);
        for (auto it = m_DestroyTextures.begin(); it != m_DestroyTextures.end();)
        {
            if (it->pBinding->nRefCount == 0)
            {
                if (it->pBinding && 
                    it->pBinding->pData)
                    interfaces::iResourceHandleUtils->deleteResource(it->pBinding);
                it = m_DestroyTextures.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    if (m_PendingReloadVisible.load(std::memory_order_acquire))
    {
        m_PendingReloadVisible.store(false, std::memory_order_release);
        if (!reloadVisible())
        {
            error.message = "Failed to load visible material";
            error.pending.store(true, std::memory_order_release);
        }
    }

    if (m_PendingReloadInvisible.load(std::memory_order_acquire))
    {
        m_PendingReloadInvisible.store(false, std::memory_order_release);
        if (!reloadInvisible())
        {
            error.message = "Failed to load invisible material";
            error.pending.store(true, std::memory_order_release);
        }
    }
}

bool __fastcall features::chams::hkDrawArray(CSceneAnimatableObjectDesc* desc, void* a2, CMeshDrawPrimitive_t* mesh_draw, int a4, void* a5, void* a6, void* a7)
{
    static auto oDrawArray = hooks::getOriginal<
        bool(__fastcall*)(CSceneAnimatableObjectDesc*, void*, CMeshDrawPrimitive_t*, int, void*, void*, void*)>
        ("DrawArray");

    onDrawArray(desc, a2, mesh_draw, a4, a5, a6, a7);

    return oDrawArray(desc, a2, mesh_draw, a4, a5, a6, a7);
}

void features::chams::onDrawArray(CSceneAnimatableObjectDesc* desc, void* a2, CMeshDrawPrimitive_t* mesh_draw, int a4, void* a5, void* a6, void* a7)
{
    static auto oDrawArray = hooks::getOriginal<
        bool(__fastcall*)(CSceneAnimatableObjectDesc*, void*, CMeshDrawPrimitive_t*, int, void*, void*, void*)>
        ("DrawArray");

    C_BaseEntity* ent = mesh_draw->pSceneAnimatableObject->hOwner.get();
    if (!ent)
        return;

    if (players_only)
    {
        static CSchemaClassInfoData* player_pawn_class = nullptr;
        if (!player_pawn_class)
            interfaces::iSchemaSystem->FindTypeScopeForModule("client.dll")->FindDeclaredClass(&player_pawn_class, "C_CSPlayerPawn");

        if (ent->getSchemaClassInfo() != player_pawn_class ||
            ent->m_lifeState() != ELifeState::ALIVE)
            return;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);

    CMaterial2* old_mat1 = mesh_draw->pMaterial;
    CMaterial2* old_mat2 = mesh_draw->pMaterial2;
    Color_t old_col = mesh_draw->colValue;

    if (invisible &&
        m_CurrentMaterialInvisible.mat.pBinding)
    {
        mesh_draw->pMaterial = m_CurrentMaterialInvisible.mat;
        mesh_draw->pMaterial2 = m_CurrentMaterialInvisible.mat;
        mesh_draw->colValue = &invisible_color.Value.x;

        // render invisible layer
        oDrawArray(desc, a2, mesh_draw, a4, a5, a6, a7);
    }

    if (visible &&
        m_CurrentMaterialVisible.mat.pBinding)
    {
        mesh_draw->pMaterial = m_CurrentMaterialVisible.mat;
        mesh_draw->pMaterial2 = m_CurrentMaterialVisible.mat;
        mesh_draw->colValue = &visible_color.Value.x;
    }
    else if (invisible)
    {
        mesh_draw->pMaterial = old_mat1;
        mesh_draw->pMaterial2 = old_mat2;
        mesh_draw->colValue = old_col;
    }
}

void features::chams::resetVisible()
{
    // visible
    if (m_CurrentMaterialVisible.mat.pBinding &&
        m_CurrentMaterialVisible.mat.pBinding != m_DefaultMaterialVisible.pBinding)
    {
        assert(m_CurrentMaterialVisible.mat.pBinding->nRefCount == 0);
        interfaces::iResourceHandleUtils->deleteResource(m_CurrentMaterialVisible.mat.pBinding);
        m_CurrentMaterialVisible.mat.pBinding = nullptr;
    }
    for (auto& handle : m_CurrentMaterialVisible.loaded_textures)
        if (handle.need_destroy &&
            handle.tex.pBinding &&
            handle.tex.pBinding->pData)
        {
            if (handle.tex.pBinding->nRefCount != 0)
            {
                std::lock_guard<std::mutex> lock(m_DestroyMutex);
                m_DestroyTextures.push_back(handle.tex);
            }
            else
            {
                interfaces::iResourceHandleUtils->deleteResource(handle.tex.pBinding);
                handle.tex.pBinding = nullptr;
            }
        }
    m_CurrentMaterialVisible.loaded_textures.clear();
}

void features::chams::resetInvisible()
{
    if (m_CurrentMaterialInvisible.mat.pBinding &&
        m_CurrentMaterialInvisible.mat.pBinding != m_DefaultMaterialInvisible.pBinding)
    {
        assert(m_CurrentMaterialInvisible.mat.pBinding->nRefCount == 0);
        interfaces::iResourceHandleUtils->deleteResource(m_CurrentMaterialInvisible.mat.pBinding);
        m_CurrentMaterialInvisible.mat.pBinding = nullptr;
    }
    for (auto& handle : m_CurrentMaterialInvisible.loaded_textures)
        if (handle.need_destroy &&
            handle.tex.pBinding &&
            handle.tex.pBinding->pData)
        {
            if (handle.tex.pBinding->nRefCount != 0)
            {
                std::lock_guard<std::mutex> lock(m_DestroyMutex);
                m_DestroyTextures.push_back(handle.tex);
            }
            else
            {
                interfaces::iResourceHandleUtils->deleteResource(handle.tex.pBinding);
                handle.tex.pBinding = nullptr;
            }
        }
    m_CurrentMaterialInvisible.loaded_textures.clear();
}