#include "Renderer.h"
#include "utils.h"

#include <d3dcompiler.h>
#include "directxtk/DDSTextureLoader.h"

#include <chrono>
#include <string>
#include <cmath>

Renderer& Renderer::GetInstance()
{
     static Renderer instance;
     return instance;
}

bool Renderer::Init(const HWND hWnd, std::shared_ptr<const Camera> pCamera)
{
     this->pCamera = pCamera;

     RECT rc;
     GetClientRect(hWnd, &rc);
     width = rc.right - rc.left;   
     height = rc.bottom - rc.top;

     IDXGIFactory* pFactory = nullptr;
     HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
     IDXGIAdapter* pSelectedAdapter = NULL;
     if (SUCCEEDED(result))
     {
          IDXGIAdapter* pAdapter = NULL;
          UINT adapterIdx = 0;
          while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter)))
          {
               DXGI_ADAPTER_DESC desc;
               pAdapter->GetDesc(&desc);
               if (wcscmp(desc.Description, L"Microsoft Basic Render Driver"))
               {
                    pSelectedAdapter = pAdapter;
                    break;
               }
               pAdapter->Release();
               adapterIdx++;
          }
     }
     if (NULL == pSelectedAdapter)
          return false;

     D3D_FEATURE_LEVEL level;
     D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
     UINT flags = 0;
#ifdef _DEBUG
     flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
     result = D3D11CreateDevice(
          pSelectedAdapter,
          D3D_DRIVER_TYPE_UNKNOWN,
          NULL,
          flags,
          levels,
          1,
          D3D11_SDK_VERSION,
          &pDevice,
          &level,
          &pDeviceContext
     );
     if (D3D_FEATURE_LEVEL_11_0 != level || !SUCCEEDED(result))
          return false;

     DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
     swapChainDesc.BufferCount = 2;
     swapChainDesc.BufferDesc.Width = width;
     swapChainDesc.BufferDesc.Height = height;
     swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
     swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
     swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
     swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
     swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
     swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
     swapChainDesc.OutputWindow = hWnd;
     swapChainDesc.SampleDesc.Count = 1;
     swapChainDesc.SampleDesc.Quality = 0;
     swapChainDesc.Windowed = true;
     swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
     swapChainDesc.Flags = 0;
     result = pFactory->CreateSwapChain(pDevice, &swapChainDesc, &pSwapChain);
     if (!SUCCEEDED(result))
          return false;

     ID3D11Texture2D* pBackBuffer = NULL;
     result = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
     if (!SUCCEEDED(result))
          return false;
     result = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pBackBufferRTV);
     if (!SUCCEEDED(result))
          return false;
     SAFE_RELEASE(pBackBuffer);

     result = CompileShaders();
     if (!SUCCEEDED(result))
          return false;

     result = CreateVertexBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateIndexBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateWorldMatrixBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateSceneMatrixBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateRasterizerState();
     if (!SUCCEEDED(result))
          return false;

     result = CreateSamplers();
     if (!SUCCEEDED(result))
          return false;

     result = CreateTextures();
     if (!SUCCEEDED(result))
          return false;

     result = CreateDepthBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateDepthState();
     if (!SUCCEEDED(result))
          return false;

     result = CreateWorldBufferInstVis();
     if (!SUCCEEDED(result))
          return false;

     result = postProc.Init(pDevice, pDeviceContext);
     if (!SUCCEEDED(result))
          return false;

     result = InitRenderTargetTexture();
     if (!SUCCEEDED(result))
          return false;

     lights.Add(
          {
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(4.0f, 1.5f, 4.0f * std::sin(milliseconds / 1000.0f), 0.0f);
               },
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(1.0f, 1.0f, std::sin(milliseconds / 100.0f), 1.0f);
               }
          });
     lights.Add(
          {
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(0.0f, -1.5f, 2.0f * std::sin(milliseconds / 300.0f), 0.0f);
               },
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(1.0f, std::sin(milliseconds / 1000.0f), 1.0f, 1.0f);
               }
          });
     lights.Add(
          {
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(1.5f, 0.0f, 2.0f, 0.0f);
               },
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
               }
          });
     lights.Add(
          {
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(0.0f, 2.0f, 0.0f, 0.0f);
               },
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
               }
          });
     lights.Add(
          {
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(std::cos(milliseconds / 2000.0f), std::sin(milliseconds / 500.0f), 0.0f, 0.0f);
               },
               [](std::size_t milliseconds)
               {
                    return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
               }
          });

     worldMatricies.reserve(maxInst);
     double deltaAngle = DirectX::XM_2PI / maxInst;
     double r = 5.0;
     for (size_t idx = 0; idx < maxInst; ++idx)
     {
          WorldMatrixBuffer worldMatrixBuffer;
          double angle = deltaAngle * idx;
          worldMatrixBuffer.worldMatrix = DirectX::XMMatrixTranslation(r*std::sin(angle), 0.0f*idx, r*std::cos(angle));
          worldMatrixBuffer.shine.x = 0.1+0.1*idx;
          worldMatrixBuffer.shine.z = float(idx % 2);
          worldMatricies.push_back(std::move(worldMatrixBuffer));
     }

     frustum.Init(0.1f);

     return sky.Init(pDevice, pDeviceContext, width, height)
          && trans.Init(pDevice, pDeviceContext, width, height);
}

bool Renderer::Render()
{
     static const FLOAT backColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

     pDeviceContext->ClearState();

     D3D11_VIEWPORT viewport;
     viewport.TopLeftX = 0;
     viewport.TopLeftY = 0;
     viewport.Width = (FLOAT)width;
     viewport.Height = (FLOAT)height;
     viewport.MinDepth = 0.0f;
     viewport.MaxDepth = 1.0f;
     pDeviceContext->RSSetViewports(1, &viewport);

     D3D11_RECT rect;
     rect.left = 0;
     rect.top = 0;
     rect.right = width;
     rect.bottom = height;
     pDeviceContext->RSSetScissorRects(1, &rect);
    

     pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthBufferDSV);
     pDeviceContext->ClearRenderTargetView(pRenderTargetView, backColor);
     pDeviceContext->ClearDepthStencilView(pDepthBufferDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);

     pDeviceContext->OMSetDepthStencilState(pDepthState, 0);

     pDeviceContext->RSSetState(pRasterizerState);

     ID3D11SamplerState* samplers[] = { pCubeTextureSampler, pCubeNormalsSampler };
     pDeviceContext->PSSetSamplers(0, 2, samplers);

     ID3D11ShaderResourceView* resources[] = { pTextureView, pCubeNormalMap };
     pDeviceContext->PSSetShaderResources(0, 2, resources);

     pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
     ID3D11Buffer* vertexBuffers[] = { pVertexBuffer };
     UINT strides[] = { sizeof(Vertex) };
     UINT offsets[] = { 0 };
     pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
     pDeviceContext->IASetInputLayout(pInputLayout);
     pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     pDeviceContext->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer);
     pDeviceContext->VSSetConstantBuffers(1, 1, &pViewMatrixBuffer);
     pDeviceContext->VSSetConstantBuffers(2, 1, &pWorldBufferInstVis);
     pDeviceContext->PSSetConstantBuffers(0, 1, &pWorldMatrixBuffer);
     pDeviceContext->PSSetConstantBuffers(1, 1, &pViewMatrixBuffer);
     pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
     pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
     
     pDeviceContext->DrawIndexedInstanced(36, ids.size(), 0, 0, 0);

     sky.Render();

     trans.Render();

     ID3D11RenderTargetView* views[] = { pBackBufferRTV };
     pDeviceContext->OMSetRenderTargets(1, views, pDepthBufferDSV);

     pDeviceContext->ClearRenderTargetView(pBackBufferRTV, backColor);
     pDeviceContext->ClearDepthStencilView(pDepthBufferDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);

     postProc.Render(viewport_, pShaderResourceViewRenderResult, pBackBufferRTV);

     HRESULT result = pSwapChain->Present(0, 0);
     return SUCCEEDED(result);
}

bool Renderer::Update()
{
     static size_t start = 
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
     std::size_t t =
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - start;

     float angle = static_cast<float>(t) / 1000;

     DirectX::XMMATRIX view = pCamera->GetViewMatrix();
     DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 3, width / (FLOAT)height, 100.0f, 0.1f);

     pDeviceContext->UpdateSubresource(pWorldMatrixBuffer, 0, nullptr, worldMatricies.data(), 0, 0);

     ids.clear();
     ids.reserve(worldMatricies.size());
     frustum.ConstructFrustum(view, proj);
     for (int i = 0; i < worldMatricies.size(); ++i)
     {
          XMFLOAT4 min, max;
          XMStoreFloat4(&min, XMVector4Transform(XMLoadFloat4(&AABB[0]), worldMatricies[i].worldMatrix));
          XMStoreFloat4(&max, XMVector4Transform(XMLoadFloat4(&AABB[1]), worldMatricies[i].worldMatrix));
          if (frustum.CheckRectangle(max.x, max.y, max.z, min.x, min.y, min.z))
          {
               ids.push_back(XMINT4(i, 0, 0, 0));
          }
     }
     pDeviceContext->UpdateSubresource(pWorldBufferInstVis, 0, nullptr, ids.data(), 0, 0);

     SceneBuffer sceneBuffer;
     sceneBuffer.viewProjMatrix = DirectX::XMMatrixMultiply(view, proj);
     DirectX::XMFLOAT3 pov = pCamera->GetPosition();
     sceneBuffer.cameraPosition.x = pov.x;
     sceneBuffer.cameraPosition.y = pov.y;
     sceneBuffer.cameraPosition.z = pov.z;
     sceneBuffer.lightCount[0] = static_cast<int>(lights.GetNumber());
     sceneBuffer.lightCount[1] = 1;
     sceneBuffer.lightCount[2] = 0;
     auto lightPositions = lights.GetPositions(t);
     std::copy(lightPositions.begin(), lightPositions.end(), sceneBuffer.lightPositions);
     auto lightColors = lights.GetColors(t);
     std::copy(lightColors.begin(), lightColors.end(), sceneBuffer.lightColors);
     sceneBuffer.ambientColor = ambientColor_;
     pDeviceContext->UpdateSubresource(pViewMatrixBuffer, 0, NULL, &sceneBuffer, 0, 0);

     return sky.Update(view, proj, pov);
}

HRESULT Renderer::SetupBackBuffer() 
{
     ID3D11Texture2D* pBackBuffer = NULL;
     HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
     if (SUCCEEDED(hr)) 
     {
          hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pBackBufferRTV);
          SAFE_RELEASE(pBackBuffer);
     }
     if (SUCCEEDED(hr))
     {
          SAFE_RELEASE(pDepthBuffer);
          SAFE_RELEASE(pDepthBufferDSV);
          D3D11_TEXTURE2D_DESC desc = {};
          desc.Format = DXGI_FORMAT_D32_FLOAT;
          desc.ArraySize = 1;
          desc.MipLevels = 1;
          desc.Usage = D3D11_USAGE_DEFAULT;
          desc.Height = height;
          desc.Width = width;
          desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
          desc.CPUAccessFlags = 0;
          desc.MiscFlags = 0;
          desc.SampleDesc.Count = 1;
          desc.SampleDesc.Quality = 0;

          hr = pDevice->CreateTexture2D(&desc, NULL, &pDepthBuffer);
          if (SUCCEEDED(hr)) {
               hr = pDevice->CreateDepthStencilView(pDepthBuffer, NULL, &pDepthBufferDSV);
          }
          return hr;
     }
}

HRESULT Renderer::CompileShaders()
{
     ID3D10Blob* vertexShaderBuffer = nullptr;
     ID3D10Blob* pixelShaderBuffer = nullptr;

     HRESULT hr = CompileShaderFromFile(L"vertex_shader.hlsl", "main", "vs_5_0", &vertexShaderBuffer);
     if (!SUCCEEDED(hr))
          return hr;
     
     hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), 
          vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader);
     if (!SUCCEEDED(hr))
          return hr;
    
     hr = CompileShaderFromFile(L"pixel_shader.hlsl", "main", "ps_5_0", &pixelShaderBuffer);
     if (!SUCCEEDED(hr))
          return hr;
     
     hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), 
          pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader);
     if (!SUCCEEDED(hr))
          return hr;

     static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
     };
     int numElements = ARRAYSIZE(InputDesc);
     hr = pDevice->CreateInputLayout(InputDesc, numElements, 
          vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout);
     
     SAFE_RELEASE(vertexShaderBuffer);
     SAFE_RELEASE(pixelShaderBuffer);

     return hr;
}

HRESULT Renderer::CreateVertexBuffer()
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

HRESULT Renderer::CreateIndexBuffer()
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

HRESULT Renderer::CreateWorldMatrixBuffer()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(WorldMatrixBuffer) * maxInst;
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     return pDevice->CreateBuffer(&desc, NULL, &pWorldMatrixBuffer);
}

HRESULT Renderer::CreateWorldBufferInstVis()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(XMUINT4)*100;
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     return pDevice->CreateBuffer(&desc, NULL, &pWorldBufferInstVis);
}

HRESULT Renderer::CreateSceneMatrixBuffer()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(SceneBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     return pDevice->CreateBuffer(&desc, NULL, &pViewMatrixBuffer);
}

HRESULT Renderer::CreateRasterizerState()
{
     D3D11_RASTERIZER_DESC rasterizeDesc;
     rasterizeDesc.AntialiasedLineEnable = false;
     rasterizeDesc.FillMode = D3D11_FILL_SOLID;
     rasterizeDesc.CullMode = D3D11_CULL_BACK;
     rasterizeDesc.DepthBias = 0;
     rasterizeDesc.DepthBiasClamp = 0.0f;
     rasterizeDesc.FrontCounterClockwise = false;
     rasterizeDesc.DepthClipEnable = true;
     rasterizeDesc.ScissorEnable = false;
     rasterizeDesc.MultisampleEnable = false;
     rasterizeDesc.SlopeScaledDepthBias = 0.0f;

     return pDevice->CreateRasterizerState(&rasterizeDesc, &pRasterizerState);
}

HRESULT Renderer::CreateTextures()
{
     if (!SUCCEEDED(DirectX::CreateDDSTextureFromFile(pDevice, L"textures/brick_normal.dds", nullptr, &pCubeNormalMap)))
          return S_FALSE;

     constexpr int n = 2;
     ID3D11Texture2D* textures[n];
     if (!SUCCEEDED(DirectX::CreateDDSTextureFromFile(pDevice, L"textures/brick.dds", (ID3D11Resource**)&textures[0], nullptr))
          || !SUCCEEDED(DirectX::CreateDDSTextureFromFile(pDevice, L"textures/eye.dds", (ID3D11Resource**)&textures[1], nullptr)))
     {
          return S_FALSE;
     }

     D3D11_TEXTURE2D_DESC textureDesc;
     textures[0]->GetDesc(&textureDesc);

     D3D11_TEXTURE2D_DESC arrayDesc;
     arrayDesc.Width = textureDesc.Width;
     arrayDesc.Height = textureDesc.Height;
     arrayDesc.MipLevels = textureDesc.MipLevels;
     arrayDesc.ArraySize = n;
     arrayDesc.Format = textureDesc.Format;
     arrayDesc.SampleDesc.Count = 1;
     arrayDesc.SampleDesc.Quality = 0;
     arrayDesc.Usage = D3D11_USAGE_DEFAULT;
     arrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
     arrayDesc.CPUAccessFlags = 0;
     arrayDesc.MiscFlags = 0;

     ID3D11Texture2D* textureArray = nullptr;
     if (FAILED(pDevice->CreateTexture2D(&arrayDesc, 0, &textureArray)))
     {
          return S_FALSE;
     }

     for (UINT texElement = 0; texElement < n; ++texElement) 
     {
          for (UINT mipLevel = 0; mipLevel < textureDesc.MipLevels; ++mipLevel)
          {
               const int sourceSubresource = D3D11CalcSubresource(mipLevel, 0, textureDesc.MipLevels);
               const int destSubresource = D3D11CalcSubresource(mipLevel, texElement, textureDesc.MipLevels);
               pDeviceContext->CopySubresourceRegion(textureArray, destSubresource, 0, 0, 0, textures[texElement], sourceSubresource, nullptr);
          }
     }

     D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
     viewDesc.Format = arrayDesc.Format;
     viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
     viewDesc.Texture2DArray.MostDetailedMip = 0;
     viewDesc.Texture2DArray.MipLevels = arrayDesc.MipLevels;
     viewDesc.Texture2DArray.FirstArraySlice = 0;
     viewDesc.Texture2DArray.ArraySize = n;

     if (FAILED(pDevice->CreateShaderResourceView(textureArray, &viewDesc, &pTextureView)))
     {
          return S_FALSE;
     }
     textureArray->Release();
     for (UINT i = 0; i < n; ++i) {
          textures[i]->Release();
     }

     return S_OK;
}

HRESULT Renderer::CreateSamplers()
{
     D3D11_SAMPLER_DESC desc = 
     { 
          D3D11_FILTER_ANISOTROPIC,
          D3D11_TEXTURE_ADDRESS_CLAMP,
          D3D11_TEXTURE_ADDRESS_CLAMP,
          D3D11_TEXTURE_ADDRESS_CLAMP,
          0.0f,
          16,
          D3D11_COMPARISON_NEVER,
          {1.0f, 1.0f, 1.0f, 1.0f},
          -D3D11_FLOAT32_MAX,
          D3D11_FLOAT32_MAX 
     };
     
     if (SUCCEEDED(pDevice->CreateSamplerState(&desc, &pCubeTextureSampler)) &&
          SUCCEEDED(pDevice->CreateSamplerState(&desc, &pCubeNormalsSampler)))
          return S_OK;
     return S_FALSE;
}

HRESULT Renderer::CreateDepthBuffer()
{
     D3D11_TEXTURE2D_DESC desc = {};
     desc.Format = DXGI_FORMAT_D16_UNORM;
     desc.ArraySize = 1;
     desc.MipLevels = 1;
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.Height = height;
     desc.Width = width;
     desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.SampleDesc.Count = 1;
     desc.SampleDesc.Quality = 0;

     HRESULT hr = pDevice->CreateTexture2D(&desc, NULL, &pDepthBuffer);
     if (SUCCEEDED(hr)) 
     {
          hr = pDevice->CreateDepthStencilView(pDepthBuffer, NULL, &pDepthBufferDSV);
     }
     return hr;
}

HRESULT Renderer::CreateDepthState()
{
     D3D11_DEPTH_STENCIL_DESC desc = { 0 };
     desc.DepthEnable = TRUE;
     desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
     desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
     desc.StencilEnable = FALSE;

     return pDevice->CreateDepthStencilState(&desc, &pDepthState);
}

HRESULT Renderer::InitRenderTargetTexture()
{
     D3D11_TEXTURE2D_DESC textureDesc = {};
     textureDesc.Width = width;
     textureDesc.Height = height;
     textureDesc.MipLevels = 1;
     textureDesc.ArraySize = 1;
     textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
     textureDesc.SampleDesc.Count = 1;
     textureDesc.Usage = D3D11_USAGE_DEFAULT;
     textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
     textureDesc.CPUAccessFlags = 0;
     textureDesc.MiscFlags = 0;

     HRESULT hr = pDevice->CreateTexture2D(&textureDesc, NULL, &pRenderTargetTexture);
     if (FAILED(hr)) {
          return hr;
     }

     D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
     renderTargetViewDesc.Format = textureDesc.Format;
     renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
     renderTargetViewDesc.Texture2D.MipSlice = 0;

     hr = pDevice->CreateRenderTargetView(pRenderTargetTexture, &renderTargetViewDesc, &pRenderTargetView);
     if (FAILED(hr)) {
          return hr;
     }

     D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
     shaderResourceViewDesc.Format = textureDesc.Format;
     shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
     shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
     shaderResourceViewDesc.Texture2D.MipLevels = 1;

     hr = pDevice->CreateShaderResourceView(pRenderTargetTexture, &shaderResourceViewDesc, &pShaderResourceViewRenderResult);
     if (FAILED(hr)) {
          return hr;
     }

     viewport_.Width = (FLOAT)width;
     viewport_.Height = (FLOAT)height;
     viewport_.MinDepth = 0.0f;
     viewport_.MaxDepth = 1.0f;
     viewport_.TopLeftX = 0;
     viewport_.TopLeftY = 0;

     return S_OK;
}

bool Renderer::Resize(const unsigned width, const unsigned height)
{
     if (width != this->width || height != this->height) {
          SAFE_RELEASE(pBackBufferRTV);

          HRESULT hr = pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
          if (SUCCEEDED(hr)) {
               this->width = width;
               this->height = height;

               hr = SetupBackBuffer();
               sky.Resize(width, height);

               if (pShaderResourceViewRenderResult) {
                    pShaderResourceViewRenderResult->Release();
                    pShaderResourceViewRenderResult = nullptr;
               }

               if (pRenderTargetView) {
                    pRenderTargetView->Release();
                    pRenderTargetView = nullptr;
               }

               if (pRenderTargetTexture) {
                    pRenderTargetTexture->Release();
                    pRenderTargetTexture = nullptr;
               }
               ZeroMemory(&viewport_, sizeof(D3D11_VIEWPORT));
               InitRenderTargetTexture();
          }
          return SUCCEEDED(hr);
     }

     return true;
}

void Renderer::Cleanup() 
{
     if (pDeviceContext)
     {
          pDeviceContext->ClearState();
     }
     SAFE_RELEASE(pBackBufferRTV);
     SAFE_RELEASE(pDeviceContext);
     SAFE_RELEASE(pSwapChain);
     SAFE_RELEASE(pDevice);
     SAFE_RELEASE(pVertexBuffer);
     SAFE_RELEASE(pIndexBuffer);
     SAFE_RELEASE(pInputLayout);
     SAFE_RELEASE(pVertexShader);
     SAFE_RELEASE(pPixelShader);
     SAFE_RELEASE(pRasterizerState);
     SAFE_RELEASE(pViewMatrixBuffer);
     SAFE_RELEASE(pWorldMatrixBuffer);
     SAFE_RELEASE(pCubeTextureSampler);
     SAFE_RELEASE(pTextureView);
     SAFE_RELEASE(pCubeNormalsSampler);
     SAFE_RELEASE(pCubeTexture);
     SAFE_RELEASE(pCubeNormalMap);
     SAFE_RELEASE(pDepthBuffer);
     SAFE_RELEASE(pDepthBufferDSV);
     SAFE_RELEASE(pDepthState);
     SAFE_RELEASE(pRenderTargetTexture);
     SAFE_RELEASE(pRenderTargetView);
     SAFE_RELEASE(pShaderResourceViewRenderResult);
     SAFE_RELEASE(pWorldBufferInstVis);
}

Renderer::~Renderer() {
     Cleanup();
}
