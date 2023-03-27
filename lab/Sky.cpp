#include "Sky.h"

#include "directxtk/DDSTextureLoader.h"
#include "utils.h"

#include <d3dcompiler.h>
#include <dxgi.h>

#include <vector>

struct SceneBuffer
{
     DirectX::XMMATRIX viewProjMatrix;
     DirectX::XMFLOAT4 cameraPos;
};

struct WorldMatrixBuffer
{
     DirectX::XMMATRIX worldMatrix;
     DirectX::XMFLOAT4 size;
};

bool Sky::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, int width, int height)
{
     this->pDevice = pDevice;
     this->pDeviceContext = pDeviceContext;
  
     HRESULT result = CreateSphere(10, 10);
     if (!SUCCEEDED(result))
          return false;
     
     result = CompileShaders();
     if (!SUCCEEDED(result))
          return false;

     result = CreateRasterizerState();
     if (!SUCCEEDED(result))
          return false;

     result = CreateSampler();
     if (!SUCCEEDED(result))
          return false;

     result = CreateTexture();
     if (!SUCCEEDED(result))
          return false;

     result = CreateWorldMatrixBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateSceneMatrixBuffer();
     if (!SUCCEEDED(result))
          return false;

     Resize(width, height);

     return true;
}

void Sky::Render()
{
     pDeviceContext->RSSetState(pRasterizerState);

     ID3D11SamplerState* samplers[] = { pSamplerState };
     pDeviceContext->PSSetSamplers(0, 1, samplers);

     ID3D11ShaderResourceView* resources[] = { pTextureView };
     pDeviceContext->PSSetShaderResources(0, 1, resources);

     pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
     ID3D11Buffer* vertexBuffers[] = { pVertexBuffer };
     UINT strides[] = { 12 };
     UINT offsets[] = { 0 };
     pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
     
     pDeviceContext->IASetInputLayout(pInputLayout);
     pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
     pDeviceContext->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer);
     pDeviceContext->VSSetConstantBuffers(1, 1, &pViewMatrixBuffer);
     pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);

     pDeviceContext->DrawIndexed(numSphereFaces * 3, 0, 0);
}

bool Sky::Update(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, DirectX::XMFLOAT3 cameraPos)
{
     WorldMatrixBuffer worldMatrixBuffer;

     worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();
     worldMatrixBuffer.size = DirectX::XMFLOAT4(radius, 0.0f, 0.0f, 0.0f);

     pDeviceContext->UpdateSubresource(pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

     D3D11_MAPPED_SUBRESOURCE subresource;
     HRESULT result = pDeviceContext->Map(pViewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
     if (SUCCEEDED(result))
     {
          SceneBuffer& viewBuffer = *reinterpret_cast<SceneBuffer*>(subresource.pData);
          viewBuffer.viewProjMatrix = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
          viewBuffer.cameraPos = DirectX::XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
          pDeviceContext->Unmap(pViewMatrixBuffer, 0);
     }
     return SUCCEEDED(result);
}

void Sky::Resize(int width, int height)
{
     float n = 0.1f;
     float fov = DirectX::XM_PI / 3;
     float halfW = tanf(fov / 2) * n;
     float halfH = float(height / width) * halfW;
     radius = sqrtf(n * n + halfH * halfH + halfW * halfW) * 11.1f * 2.0f;
}

void Sky::Cleanup()
{
     SAFE_RELEASE(pVertexBuffer);
     SAFE_RELEASE(pIndexBuffer);
     SAFE_RELEASE(pInputLayout);
     SAFE_RELEASE(pVertexShader);
     SAFE_RELEASE(pRasterizerState);
     SAFE_RELEASE(pViewMatrixBuffer);
     SAFE_RELEASE(pWorldMatrixBuffer);
     SAFE_RELEASE(pPixelShader);
     SAFE_RELEASE(pSamplerState);
     SAFE_RELEASE(pTextureView);
}

Sky::~Sky()
{
     Cleanup();
}

HRESULT Sky::CreateSphere(int latLines, int longLines)
{
     numSphereVertices = ((latLines - 2) * longLines) + 2;
     numSphereFaces = ((latLines - 3) * (longLines) * 2) + (longLines * 2);

     float sphereYaw = 0.0f;
     float spherePitch = 0.0f;

     std::vector<Vertex> vertices(numSphereVertices);

     DirectX::XMVECTOR currVertPos = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

     vertices[0].x = 0.0f;
     vertices[0].y = 0.0f;
     vertices[0].z = 1.0f;

     for (int i = 0; i < latLines - 2; ++i)
     {
          spherePitch = (i + 1) * (DirectX::XM_PI/ ((INT64)latLines - 1));
          DirectX::XMMATRIX Rotationx = DirectX::XMMatrixRotationX(spherePitch);
          for (int j = 0; j < longLines; ++j)
          {
               sphereYaw = j * (DirectX::XM_2PI / (longLines));
               DirectX::XMMATRIX Rotationy = DirectX::XMMatrixRotationZ(sphereYaw);
               currVertPos = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
                         (Rotationx * Rotationy));
               currVertPos = DirectX::XMVector3Normalize(currVertPos);
               vertices[(INT64)i * longLines + j + 1].x = DirectX::XMVectorGetX(currVertPos);
               vertices[(INT64)i * longLines + j + 1].y = DirectX::XMVectorGetY(currVertPos);
               vertices[(INT64)i * longLines + j + 1].z = DirectX::XMVectorGetZ(currVertPos);
          }
     }

     vertices[(INT64)numSphereVertices - 1].x = 0.0f;
     vertices[(INT64)numSphereVertices - 1].y = 0.0f;
     vertices[(INT64)numSphereVertices - 1].z = -1.0f;


     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(Vertex) * numSphereVertices;
     desc.Usage = D3D11_USAGE_IMMUTABLE;
     desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     D3D11_SUBRESOURCE_DATA data;
     ZeroMemory(&data, sizeof(data));
     data.pSysMem = &vertices[0];
     data.SysMemPitch = sizeof(Vertex) * numSphereVertices;
     data.SysMemSlicePitch = 0;

     HRESULT hr = pDevice->CreateBuffer(&desc, &data, &pVertexBuffer);

     if (!SUCCEEDED(hr))
     {
          return hr;
     }

     std::vector<DWORD> indices((INT64)numSphereFaces * 3);

     int k = 0;
     for (int l = 0; l < longLines - 1; ++l)
     {
          indices[k] = 0;
          indices[(INT64)k + 1] = l + 1;
          indices[(INT64)k + 2] = l + 2;
          k += 3;
     }

     indices[k] = 0;
     indices[(INT64)k + 1] = longLines;
     indices[(INT64)k + 2] = 1;
     k += 3;

     for (int i = 0; i < latLines - 3; ++i)
     {
          for (int j = 0; j < longLines - 1; ++j)
          {
               indices[k] = i * longLines + j + 1;
               indices[(INT64)k + 1] = i * longLines + j + 2;
               indices[(INT64)k + 2] = (i + 1) * longLines + j + 1;

               indices[(INT64)k + 3] = (i + 1) * longLines + j + 1;
               indices[(INT64)k + 4] = i * longLines + j + 2;
               indices[(INT64)k + 5] = (i + 1) * longLines + j + 2;

               k += 6; // next quad
          }

          indices[k] = (i * longLines) + longLines;
          indices[(INT64)k + 1] = (i * longLines) + 1;
          indices[(INT64)k + 2] = ((i + 1) * longLines) + longLines;

          indices[(INT64)k + 3] = ((i + 1) * longLines) + longLines;
          indices[(INT64)k + 4] = (i * longLines) + 1;
          indices[(INT64)k + 5] = ((i + 1) * longLines) + 1;

          k += 6;
     }

     for (int l = 0; l < longLines - 1; ++l)
     {
          indices[k] = numSphereVertices - 1;
          indices[(INT64)k + 1] = (numSphereVertices - 1) - (l + 1);
          indices[(INT64)k + 2] = (numSphereVertices - 1) - (l + 2);
          k += 3;
     }

     indices[k] = numSphereVertices - 1;
     indices[(INT64)k + 1] = (numSphereVertices - 1) - longLines;
     indices[(INT64)k + 2] = numSphereVertices - 2;

     D3D11_BUFFER_DESC indexBufferDesc;
     ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

     indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
     indexBufferDesc.ByteWidth = sizeof(DWORD) * numSphereFaces * 3;
     indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
     indexBufferDesc.CPUAccessFlags = 0;
     indexBufferDesc.MiscFlags = 0;

     D3D11_SUBRESOURCE_DATA iinitData;

     iinitData.pSysMem = &indices[0];
     return pDevice->CreateBuffer(&indexBufferDesc, &iinitData, &pIndexBuffer);
}

HRESULT Sky::CompileShaders()
{
     ID3D10Blob* vertexShaderBuffer = nullptr;
     ID3D10Blob* pixelShaderBuffer = nullptr;

     int flags = 0;
#ifdef _DEBUG
     flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

     HRESULT hr = D3DCompileFromFile(L"cubemap_vertex_shader.hlsl", NULL, NULL,
          "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
          vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader);
     if (!SUCCEEDED(hr))
          return hr;

     hr = D3DCompileFromFile(L"cubemap_pixel_shader.hlsl", NULL, NULL,
          "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
          pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader);
     if (!SUCCEEDED(hr))
          return hr;

     static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
     };
     int numElements = sizeof(InputDesc) / sizeof(InputDesc[0]);
     hr = pDevice->CreateInputLayout(InputDesc, numElements,
          vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout);

     SAFE_RELEASE(vertexShaderBuffer);
     SAFE_RELEASE(pixelShaderBuffer);

     return hr;
}

HRESULT Sky::CreateTexture()
{
     return DirectX::CreateDDSTextureFromFileEx(pDevice, pDeviceContext, L"textures/sky.dds",
          0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE,
          false, nullptr, &pTextureView);
}

HRESULT Sky::CreateSampler()
{
     D3D11_SAMPLER_DESC desc = {};
     desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
     desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
     desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
     desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
     desc.MinLOD = -D3D11_FLOAT32_MAX;
     desc.MaxLOD = D3D11_FLOAT32_MAX;
     desc.MipLODBias = 0.0f;
     desc.MaxAnisotropy = 16;
     desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
     desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 1.0f;
     return pDevice->CreateSamplerState(&desc, &pSamplerState);
}

HRESULT Sky::CreateRasterizerState()
{
     D3D11_RASTERIZER_DESC rasterizeDesc;
     rasterizeDesc.AntialiasedLineEnable = false;
     rasterizeDesc.FillMode = D3D11_FILL_SOLID;
     rasterizeDesc.CullMode = D3D11_CULL_NONE;
     rasterizeDesc.DepthBias = 0;
     rasterizeDesc.DepthBiasClamp = 0.0f;
     rasterizeDesc.FrontCounterClockwise = false;
     rasterizeDesc.DepthClipEnable = true;
     rasterizeDesc.ScissorEnable = false;
     rasterizeDesc.MultisampleEnable = false;
     rasterizeDesc.SlopeScaledDepthBias = 0.0f;

     return pDevice->CreateRasterizerState(&rasterizeDesc, &pRasterizerState);
}

HRESULT Sky::CreateWorldMatrixBuffer()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(WorldMatrixBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     WorldMatrixBuffer worldMatrixBuffer;
     worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

     D3D11_SUBRESOURCE_DATA data;
     data.pSysMem = &worldMatrixBuffer;
     data.SysMemPitch = sizeof(worldMatrixBuffer);
     data.SysMemSlicePitch = 0;

     return pDevice->CreateBuffer(&desc, NULL, &pWorldMatrixBuffer);
}

HRESULT Sky::CreateSceneMatrixBuffer()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(SceneBuffer);
     desc.Usage = D3D11_USAGE_DYNAMIC;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     return pDevice->CreateBuffer(&desc, NULL, &pViewMatrixBuffer);
}
