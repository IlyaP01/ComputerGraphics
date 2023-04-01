#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "utils.h"
#include <directxmath.h>

using namespace DirectX;

class PostProc
{
public:
     HRESULT Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
     void Render(D3D11_VIEWPORT viewport, ID3D11ShaderResourceView* texture, ID3D11RenderTargetView* renderTarget);
     void CleanUp();
     ~PostProc();

private:
     ID3D11Device* pDevice = nullptr;
     ID3D11DeviceContext* pDeviceContext = nullptr;
     ID3D11VertexShader* pVertexShader = nullptr;
     ID3D11PixelShader* pPixelShader = nullptr;
     ID3D11SamplerState* pSamplerState = nullptr;
};

