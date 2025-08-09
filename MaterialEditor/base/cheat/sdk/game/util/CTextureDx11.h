#pragma once
#include <d3d11.h>
#include <util/mem.h>
#include <cheat/sdk/sdk_object.h>


class CTextureDx11
{
    SDK_CLASS(CTextureDx11);
public:
    MEM_PAD(0x10); // 0x0
    ID3D11ShaderResourceView* m_pTextureSRV0; // 0x10
    ID3D11ShaderResourceView* m_pTextureSRV1; // 0x18
};