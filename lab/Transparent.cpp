#include "Transparent.h"

#include <d3dcompiler.h>
#include <dxgi.h>

#define SAFE_RELEASE(DXResource) do { if ((DXResource) != NULL) { (DXResource)->Release(); } } while (false);

bool Transparent::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, int width, int height)
{
     this->pDevice = pDevice;
     this->pDeviceContext = pDeviceContext;
     HRESULT result = CompileShaders();
     if (!SUCCEEDED(result))
          return false;

     result = CreateVertexBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateIndexBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateRasterizerState();
     if (!SUCCEEDED(result))
          return false;

     result = CreateBlendState();
     if (!SUCCEEDED(result))
          return false;

     result = CreateDepthState();
     if (!SUCCEEDED(result))
          return false;

     return true;
}

void Transparent::Render()
{
     pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
     ID3D11Buffer* vertexBuffers[] = { pVertexBuffer };
     UINT stride = sizeof(DirectX::XMFLOAT3);
     UINT offset = 0;
     pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
     pDeviceContext->IASetInputLayout(pInputLayout);

     pDeviceContext->RSSetState(pRasterizerState);
     pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     pDeviceContext->RSSetState(pRasterizerState);

     pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);

     pDeviceContext->OMSetBlendState(pBlendState, nullptr, 0xFFFFFFFF);
     pDeviceContext->OMSetDepthStencilState(pDepthState, 0);

     pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
     pDeviceContext->DrawIndexed(6, 0, 0);

     pDeviceContext->VSSetShader(pVertexShader2, nullptr, 0);
     pDeviceContext->DrawIndexed(6, 0, 0);

}

void Transparent::Cleanup()
{
     SAFE_RELEASE(pVertexBuffer);
     SAFE_RELEASE(pIndexBuffer);
     SAFE_RELEASE(pVertexShader);
     SAFE_RELEASE(pPixelShader);
     SAFE_RELEASE(pRasterizerState);
     SAFE_RELEASE(pInputLayout);
     SAFE_RELEASE(pDepthState);
     SAFE_RELEASE(pBlendState);
}

Transparent::~Transparent()
{
     Cleanup();
}

HRESULT Transparent::CreateVertexBuffer()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(vertices);
     desc.Usage = D3D11_USAGE_IMMUTABLE;
     desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     D3D11_SUBRESOURCE_DATA data;
     data.pSysMem = &vertices;
     data.SysMemPitch = sizeof(vertices);
     data.SysMemSlicePitch = 0;

     return pDevice->CreateBuffer(&desc, &data, &pVertexBuffer);
}

HRESULT Transparent::CreateIndexBuffer()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(indices);
     desc.Usage = D3D11_USAGE_IMMUTABLE;
     desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     D3D11_SUBRESOURCE_DATA data;
     data.pSysMem = &indices;
     data.SysMemPitch = sizeof(indices);
     data.SysMemSlicePitch = 0;

     return pDevice->CreateBuffer(&desc, &data, &pIndexBuffer);
}

HRESULT Transparent::CompileShaders()
{
     ID3D10Blob* vertexShaderBuffer = nullptr;
     ID3D10Blob* vertexShaderBuffer2 = nullptr;
     ID3D10Blob* pixelShaderBuffer = nullptr;

     int flags = 0;
#ifdef _DEBUG
     flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

     HRESULT hr = D3DCompileFromFile(L"TransVS1.hlsl", NULL, NULL,
          "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
          vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader);
     if (!SUCCEEDED(hr))
          return hr;

     hr = D3DCompileFromFile(L"TransVS2.hlsl", NULL, NULL,
          "main", "vs_5_0", flags, 0, &vertexShaderBuffer2, NULL);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreateVertexShader(vertexShaderBuffer2->GetBufferPointer(),
          vertexShaderBuffer2->GetBufferSize(), NULL, &pVertexShader2);
     if (!SUCCEEDED(hr))
          return hr;

     hr = D3DCompileFromFile(L"TransPS.hlsl", NULL, NULL,
          "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
          pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader);
     if (!SUCCEEDED(hr))
          return hr;

     static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
     };
     int numElements = sizeof(InputDesc) / sizeof(InputDesc[0]);
     hr = pDevice->CreateInputLayout(InputDesc, numElements,
          vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout);

     SAFE_RELEASE(vertexShaderBuffer);
     SAFE_RELEASE(vertexShaderBuffer2);
     SAFE_RELEASE(pixelShaderBuffer);

     return hr;
}

HRESULT Transparent::CreateRasterizerState()
{
     D3D11_RASTERIZER_DESC desc = {};
     desc.FillMode = D3D11_FILL_SOLID;
     desc.CullMode = D3D11_CULL_NONE;
     desc.FrontCounterClockwise = false;
     desc.DepthBias = 0;
     desc.SlopeScaledDepthBias = 0.0f;
     desc.DepthBiasClamp = 0.0f;
     desc.DepthClipEnable = true;
     desc.ScissorEnable = false;
     desc.MultisampleEnable = false;
     desc.AntialiasedLineEnable = false;

     return pDevice->CreateRasterizerState(&desc, &pRasterizerState);
}

HRESULT Transparent::CreateBlendState()
{
     D3D11_BLEND_DESC desc = { 0 };
     desc.AlphaToCoverageEnable = false;
     desc.IndependentBlendEnable = false;
     desc.RenderTarget[0].BlendEnable = true;
     desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
     desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
     desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
     desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED |
          D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
     desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
     desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
     desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;

     return pDevice->CreateBlendState(&desc, &pBlendState);
}

HRESULT Transparent::CreateDepthState()
{
     D3D11_DEPTH_STENCIL_DESC desc = { 0 };
     desc.DepthEnable = TRUE;
     desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
     desc.DepthFunc = D3D11_COMPARISON_GREATER;
     desc.StencilEnable = FALSE;

     return pDevice->CreateDepthStencilState(&desc, &pDepthState);
}
