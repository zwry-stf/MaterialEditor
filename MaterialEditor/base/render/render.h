#pragma once
#include <stdint.h>
#include <Windows.h>
#include <d3d11.h>
#include <optional>


class CInputSystem;

class render
{
public:
	static void init();
	static void destroy();
	static void initMouse();

	static HRESULT            hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static HRESULT __fastcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	static HRESULT __fastcall hkResizeBuffers(IDXGISwapChain* pSwapChain, uint32_t nBufferCount, uint32_t nWidth, uint32_t nHeight, DXGI_FORMAT newFormat, uint32_t nFlags);
	static HRESULT __fastcall hkCreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
	static int     __fastcall hkSetRelativeMouseMode(CInputSystem* this_ptr, bool enabled);

private:
	inline static HWND                    m_hHwnd          = NULL;
	inline static ID3D11DeviceContext*    m_pDeviceContext = nullptr;
	inline static ID3D11RenderTargetView* m_pRenderTarget  = nullptr;
	inline static bool                    m_bMenuOpen      = true;
	inline static WNDPROC                 m_pWndProc       = NULL;

private:
	static void renderMenu();
	static void onToggleMenu(bool open);
};