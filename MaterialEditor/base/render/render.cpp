#include "render.h"
#include <init/interfaces.h>
#include <cheat/sdk/interfaces/CInputSystem.h>
#include <init/hooks.h>
#include <init/signatures.h>
#include <base.h>
#include <util/mem.h>
#include <mutex>

#pragma comment(lib, "d3d11.lib")

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>


class OverlaySwapchain
{
public:
    MEM_PAD(0x18);
    IDXGISwapChain* swapchain;
    MEM_PAD(0x8);
    ID3D11Device* device;
};

inline static bool last_mouse_rel = false;

#define PRESENT_IDX 8
#define RESIZE_BUFFERS_IDX 13
#define CREATE_SWAPCHAIN_IDX 10

inline static std::mutex init_mutex;


void render::init()
{
    ID3D11Device* g_pd3dDevice = nullptr;

    static auto _swapchain = signatures::get<OverlaySwapchain**>("Swapchain");

    if (*_swapchain == nullptr || (*_swapchain)->swapchain == nullptr)
        throw std::runtime_error("failed to get swapchain");

    IDXGISwapChain* swapchain = (*_swapchain)->swapchain;

    void* present = mem::GetVFunc(swapchain, PRESENT_IDX);
    hooks::registerHook(hkPresent, present, "Present");

    void* resize_buffers = mem::GetVFunc(swapchain, RESIZE_BUFFERS_IDX);
    hooks::registerHook(hkResizeBuffers, resize_buffers, "ResizeBuffers");

    HRESULT hr = swapchain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_pd3dDevice));
    if (FAILED(hr))
        throw std::runtime_error("failed to get device");

    // Get Factory
    IDXGIFactory* dxgiFactory = nullptr;

    IDXGIDevice* dxgiDevice = nullptr;
    hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (FAILED(hr))
        throw std::runtime_error("failed to get dxgi device");

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hr))
        throw std::runtime_error("failed to get dxgi adapter");

    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));
    if (FAILED(hr))
        throw std::runtime_error("failed to get dxgi factory");

    dxgiAdapter->Release();
    dxgiDevice->Release();

    void* create_swap_chain = mem::GetVFunc(dxgiFactory, CREATE_SWAPCHAIN_IDX);
    hooks::registerHook(hkCreateSwapChain, create_swap_chain, "CreateSwapChain");

    DXGI_SWAP_CHAIN_DESC swapchainDesc;
    swapchain->GetDesc(&swapchainDesc);

    ID3D11Texture2D* backbuffer;
    hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
    if (FAILED(hr))
        throw std::runtime_error("failed to get backbuffer");

    D3D11_TEXTURE2D_DESC desc;
    backbuffer->GetDesc(&desc);

    hr = g_pd3dDevice->CreateRenderTargetView(backbuffer, NULL, &m_pRenderTarget);
    backbuffer->Release();
    if (FAILED(hr))
        throw std::runtime_error("failed to create rendertarget");

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyhbd.ttc", 16.0f);

    io.DisplaySize = ImVec2((float)desc.Width, (float)desc.Height);

    backbuffer->Release();

    m_hHwnd = swapchainDesc.OutputWindow;
    ImGui_ImplWin32_Init(swapchainDesc.OutputWindow);

    g_pd3dDevice->GetImmediateContext(&m_pDeviceContext);

    ImGui_ImplDX11_Init(g_pd3dDevice, m_pDeviceContext);
    ImGui_ImplDX11_CreateDeviceObjects();

    ImGui::GetBackgroundDrawList()->Flags |= ImDrawListFlags_AntiAliasedFill;
    ImGui::GetBackgroundDrawList()->Flags |= ImDrawListFlags_AntiAliasedLines;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.3f;
    style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 0;

    style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);

    // hook wndproc
    m_pWndProc = (WNDPROC)SetWindowLongPtrW(m_hHwnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
    if (!m_pWndProc)
        throw std::runtime_error("failed to find original wndproc");
}

void render::destroy()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (m_pWndProc != nullptr)
    {
        SetWindowLongPtrW(m_hHwnd, GWLP_WNDPROC, (LONG_PTR)m_pWndProc);
        m_pWndProc = nullptr;
    }
}

void render::initMouse()
{
    // get mouse open
    last_mouse_rel = !interfaces::iInputSystem->isRelativeMouseMode();

    static auto oSetRelativeMouseMode = hooks::getOriginal<
        int(__fastcall*)(CInputSystem*, bool)>
        ("SetRelativeMouseMode");

    if (last_mouse_rel)
        oSetRelativeMouseMode(interfaces::iInputSystem, false);
}

HRESULT __fastcall render::hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static auto oPresent = hooks::getOriginal<
        HRESULT(__fastcall*)(IDXGISwapChain*, UINT, UINT)>
        ("Present");

    m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTarget, nullptr);

    // Start frame
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();

    // Render menu
    renderMenu();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT render::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
    {
        m_bMenuOpen = !m_bMenuOpen;
        return 0;
    }
    else if (m_bMenuOpen && uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
    {
        m_bMenuOpen = false;
        return 0;
    }

    // Handle messages
    if (m_bMenuOpen)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return 0;

        if (uMsg == WM_LBUTTONDOWN ||
            uMsg == WM_LBUTTONUP ||
            uMsg == WM_MOUSEMOVE ||
            uMsg == WM_KEYDOWN ||
            uMsg == WM_MOUSEWHEEL ||
            uMsg == WM_MOUSEHWHEEL)
            return 0;
    }

    return CallWindowProcA(m_pWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __fastcall render::hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
    static auto oCreateSwapChain = hooks::getOriginal<
        HRESULT(__fastcall*)(IDXGIFactory*, IUnknown*, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**)>
        ("CreateSwapChain");

    if (m_pRenderTarget) m_pRenderTarget->Release(); m_pRenderTarget = NULL;

    return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
}

HRESULT __fastcall render::hkResizeBuffers(IDXGISwapChain* pSwapChain, uint32_t nBufferCount, uint32_t nWidth, uint32_t nHeight, DXGI_FORMAT newFormat, uint32_t nFlags)
{
    static auto oResizeBuffers = hooks::getOriginal<
        HRESULT(__fastcall*)(IDXGISwapChain*, uint32_t, uint32_t, uint32_t, DXGI_FORMAT, uint32_t)>
        ("ResizeBuffers");

    HRESULT hr = oResizeBuffers(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);

    ID3D11Device* device;
    pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device));

    device->GetImmediateContext(&m_pDeviceContext);

    ID3D11Texture2D* backbuffer;
    hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
    if (FAILED(hr))
        throw std::runtime_error("failed to get backbuffer");

    D3D11_TEXTURE2D_DESC desc;
    backbuffer->GetDesc(&desc);

    hr = device->CreateRenderTargetView(backbuffer, NULL, &m_pRenderTarget);
    backbuffer->Release();
    if (FAILED(hr))
        throw std::runtime_error("failed to create rendertarget");

    return hr;
}

int __fastcall render::hkSetRelativeMouseMode(CInputSystem* this_ptr, bool enabled)
{
    static auto oSetRelativeMouseMode = hooks::getOriginal<
        int(__fastcall*)(CInputSystem*, bool)>
        ("SetRelativeMouseMode");

    last_mouse_rel = enabled;

    if (!m_bMenuOpen)
        return oSetRelativeMouseMode(this_ptr, enabled);

    return 1;
}

static int _StdStringResizeCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* str = static_cast<std::string*>(data->UserData);
        str->resize(static_cast<size_t>(data->BufTextLen));
        data->Buf = str->data();
    }
    return 0;
}

static bool InputTextMultilineStdString(const char* label, std::string& str, const ImVec2& size, ImGuiInputTextFlags flags = 0)
{
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputTextMultiline(label, str.data(), str.size() + 1, size, flags, _StdStringResizeCallback, &str);
}

#include <cheat/features/chams/chams.h>
void render::renderMenu()
{
    static bool last_open = false;
    if (last_open != m_bMenuOpen)
    {
        last_open = m_bMenuOpen;
        onToggleMenu(m_bMenuOpen);
    }

    if (!m_bMenuOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

    ImGui::SeparatorText("Material Overrides");

    // Global toggle
    ImGui::Checkbox("Only render players", &features::chams::players_only);
    ImGui::Spacing();

    // Two panels side-by-side
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float pad = ImGui::GetStyle().ItemSpacing.x;
    float half = (avail.x - pad) * 0.5f;
    float textHeight = ImGui::GetTextLineHeightWithSpacing();

    // Left: Visible
    ImGui::BeginChild("##visible_child", ImVec2(half, 0), ImGuiChildFlags_Border);
    {
        ImGui::TextUnformatted("Visible material");
        ImGui::SameLine();
        ImGui::Checkbox("Enable##vis", &features::chams::visible);
        ImGui::SameLine();
        if (ImGui::ColorButton("##vis", features::chams::visible_color.Value))
            ImGui::OpenPopup("color_popup##vis");
        if (ImGui::BeginPopup("color_popup##vis"))
        {
            ImGui::ColorPicker4("##vis_picker", (float*)&features::chams::visible_color,
                ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
            ImGui::EndPopup();
        }
        ImGui::SameLine();

        static char visible_mat_name[256] = "materials/dev/primary_white.vmat";
        static std::string visible_mat = features::chams::default_material_visible;
        if (ImGui::Button("Update##vis"))
            features::chams::reloadVisible(visible_mat_name, visible_mat);

        ImGui::InputText("Material", visible_mat_name, sizeof(visible_mat_name));

        ImGui::Dummy(ImVec2(0, 4));
        ImVec2 boxSize = ImVec2(-1.0f, std::max(120.0f, 16.0f * textHeight));
        InputTextMultilineStdString("##visible_text", visible_mat, boxSize,
            ImGuiInputTextFlags_AllowTabInput);

        auto allocated = features::chams::getAllocatedVisible();
        ImGui::SeparatorText(("Allocated Textures (" + std::to_string(allocated.size()) + ")").c_str());
        for (auto& path : allocated)
            ImGui::TextUnformatted(path.c_str());
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Right: Invisible
    ImGui::BeginChild("##invisible_child", ImVec2(half, 0), ImGuiChildFlags_Border);
    {
        ImGui::TextUnformatted("Invisible material");
        ImGui::SameLine();
        ImGui::Checkbox("Enable##invis", &features::chams::invisible);
        ImGui::SameLine();
        if (ImGui::ColorButton("##invis", features::chams::invisible_color.Value))
            ImGui::OpenPopup("color_popup##invis");
        if (ImGui::BeginPopup("color_popup##invis")) 
        {
            ImGui::ColorPicker4("##invis_picker", (float*)&features::chams::invisible_color,
                ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        static char invisible_mat_name[256] = "materials/dev/primary_white.vmat";
        static std::string invisible_mat = features::chams::default_material_invisible;
        if (ImGui::Button("Update##invis"))
            features::chams::reloadInvisible(invisible_mat_name, invisible_mat);

        ImGui::InputText("Material", invisible_mat_name, sizeof(invisible_mat_name));

        ImGui::Dummy(ImVec2(0, 4));
        ImVec2 boxSize = ImVec2(-1.0f, std::max(120.0f, 16.0f * textHeight));
        InputTextMultilineStdString("##invisible_text", invisible_mat, boxSize,
            ImGuiInputTextFlags_AllowTabInput);

        auto allocated = features::chams::getAllocatedInvisible();
        ImGui::SeparatorText(("Allocated Textures (" + std::to_string(allocated.size()) + ")").c_str());
        for (auto& path : allocated)
            ImGui::TextUnformatted(path.c_str());
    }
    ImGui::EndChild();
}

void render::onToggleMenu(bool open)
{
    // getOriginal can fail because this gets called before hooks have been initialized
    static auto oSetRelativeMouseMode = hooks::getOriginal<
        int(__fastcall*)(CInputSystem*, bool)>
        ("SetRelativeMouseMode");

    if (open)
    {
        oSetRelativeMouseMode(interfaces::iInputSystem, false);

        POINT point = {
            static_cast<LONG>(ImGui::GetIO().DisplaySize.x * 0.5f),
            static_cast<LONG>(ImGui::GetIO().DisplaySize.y * 0.5f) };
        ClientToScreen(m_hHwnd, &point);

        SetCursorPos(point.x, point.y);
    }
    else if (last_mouse_rel)
    {
        oSetRelativeMouseMode(interfaces::iInputSystem, true);
    }
}