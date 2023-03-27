#include "Transparent.h"
#include "utils.h"
#include "Lights.h"

#include <d3dcompiler.h>
#include <dxgi.h>

struct TransparentWorldBuffer
{
     DirectX::XMMATRIX worldMatrix;
     DirectX::XMFLOAT4 color;
};

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
     
     result = CreateWorldBuffers();
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
     pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);

     pDeviceContext->OMSetBlendState(pBlendState, nullptr, 0xFFFFFFFF);
     pDeviceContext->OMSetDepthStencilState(pDepthState, 0);

     pDeviceContext->VSSetConstantBuffers(0, 1, &pWorldBuffer);
     pDeviceContext->PSSetConstantBuffers(0, 1, &pWorldBuffer);
     pDeviceContext->DrawIndexed(6, 0, 0);

     pDeviceContext->VSSetConstantBuffers(0, 1, &pWorldBuffer2);
     pDeviceContext->PSSetConstantBuffers(0, 1, &pWorldBuffer2);
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
     SAFE_RELEASE(pWorldBuffer);
     SAFE_RELEASE(pWorldBuffer2);
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
     ID3D10Blob* pixelShaderBuffer = nullptr;

     HRESULT hr = CompileShaderFromFile(L"TransVS.hlsl", "main", "vs_5_0", &vertexShaderBuffer);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
          vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader);
     if (!SUCCEEDED(hr))
          return hr;

     hr = CompileShaderFromFile(L"TransPS.hlsl", "main", "ps_5_0", &pixelShaderBuffer);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
          pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader);
     if (!SUCCEEDED(hr))
          return hr;

     static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
     };
     int numElements = ARRAYSIZE(InputDesc);
     hr = pDevice->CreateInputLayout(InputDesc, numElements,
          vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout);

     SAFE_RELEASE(vertexShaderBuffer);
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

HRESULT Transparent::CreateWorldBuffers()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(TransparentWorldBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;



     if (SUCCEEDED(pDevice->CreateBuffer(&desc, NULL, &pWorldBuffer))
          && SUCCEEDED(pDevice->CreateBuffer(&desc, NULL, &pWorldBuffer2)))
     {
          TransparentWorldBuffer transparentWorldBuffer;
          transparentWorldBuffer.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.1f);
          transparentWorldBuffer.color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.5f);
          pDeviceContext->UpdateSubresource(pWorldBuffer, 0, NULL, &transparentWorldBuffer, 0, 0);

          transparentWorldBuffer.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.1f);
          transparentWorldBuffer.color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 0.5f);
          pDeviceContext->UpdateSubresource(pWorldBuffer2, 0, NULL, &transparentWorldBuffer, 0, 0);
          return S_OK;
     }

     return S_FALSE;
}
