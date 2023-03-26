#include "PostProc.h"

HRESULT PostProc::Init(ID3D11Device* pDevice, ID3D11DeviceContext* ppDeviceContext)
{
     this-> pDevice = pDevice;
     this->pDeviceContext = ppDeviceContext;
     
     ID3D10Blob* vertexShaderBuffer = nullptr;
     ID3D10Blob* pixelShaderBuffer = nullptr;

     HRESULT hr = CompileShaderFromFile(L"postprocVS.hlsl", "main", "vs_5_0", &vertexShaderBuffer);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
          vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader);
     if (!SUCCEEDED(hr))
          return hr;

     hr = CompileShaderFromFile(L"postprocPS.hlsl", "main", "ps_5_0", &pixelShaderBuffer);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
          pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader);
     if (!SUCCEEDED(hr))
          return hr;

     SAFE_RELEASE(vertexShaderBuffer);
     SAFE_RELEASE(pixelShaderBuffer);

     D3D11_SAMPLER_DESC samplerDesc;
     ZeroMemory(&samplerDesc, sizeof(samplerDesc));
     samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
     samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
     samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
     samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
     samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
     samplerDesc.MinLOD = 0;
     samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
     samplerDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
     hr = pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
     if (!SUCCEEDED(hr))
          return hr;

     return S_OK;
}

void PostProc::Render(D3D11_VIEWPORT viewport, ID3D11ShaderResourceView* texture, ID3D11RenderTargetView* renderTarget)
{
     pDeviceContext->OMSetRenderTargets(1, &renderTarget, nullptr);
     pDeviceContext->RSSetViewports(1, &viewport);

     pDeviceContext->IASetInputLayout(nullptr);
     pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

     pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
     pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
     pDeviceContext->PSSetShaderResources(0, 1, &texture);
     pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);

     pDeviceContext->Draw(3, 0);

     ID3D11ShaderResourceView* nullsrv[] = { nullptr };
     pDeviceContext->PSSetShaderResources(0, 1, nullsrv);
}

void PostProc::CleanUp()
{
     SAFE_RELEASE(pVertexShader);
     SAFE_RELEASE(pPixelShader);
     SAFE_RELEASE(pSamplerState);
}

PostProc::~PostProc()
{
     CleanUp();
}
