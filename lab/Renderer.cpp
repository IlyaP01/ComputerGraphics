#include "Renderer.h"

#include <d3dcompiler.h>
#include "directxtk/DDSTextureLoader.h"

#include <chrono>
#include <string>

#define SAFE_RELEASE(DXResource) do { if ((DXResource) != NULL) { (DXResource)->Release(); } } while (false);

struct ViewMatrixBuffer
{
     DirectX::XMMATRIX viewProjMatrix;
};

struct WorldMatrixBuffer
{
     DirectX::XMMATRIX worldMatrix;
};

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

     result = CreateSampler();
     if (!SUCCEEDED(result))
          return false;

     result = CreateTexture();
     if (!SUCCEEDED(result))
          return false;

     result = CreateDepthBuffer();
     if (!SUCCEEDED(result))
          return false;

     result = CreateDepthState();
     if (!SUCCEEDED(result))
          return false;

     return sky.Init(pDevice, pDeviceContext, width, height) && trans.Init(pDevice, pDeviceContext, width, height);
}

bool Renderer::Render()
{
     pDeviceContext->ClearState();
     ID3D11RenderTargetView* views[] = { pBackBufferRTV };
     pDeviceContext->OMSetRenderTargets(1, views, pDepthBufferDSV);
     pDeviceContext->OMSetDepthStencilState(pDepthState, 0);

     static const FLOAT backColor[4] = { 0.5f, 0.5f, 0.7f, 1.0f };
     pDeviceContext->ClearRenderTargetView(pBackBufferRTV, backColor);
     pDeviceContext->ClearDepthStencilView(pDepthBufferDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);

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

     pDeviceContext->RSSetState(pRasterizerState);

     ID3D11SamplerState* samplers[] = { pSamplerState };
     pDeviceContext->PSSetSamplers(0, 1, samplers);

     ID3D11ShaderResourceView* resources[] = { pTextureView };
     pDeviceContext->PSSetShaderResources(0, 1, resources);

     pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
     ID3D11Buffer* vertexBuffers[] = { pVertexBuffer };
     UINT strides[] = { 20 };
     UINT offsets[] = { 0 };
     pDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
     pDeviceContext->IASetInputLayout(pInputLayout);
     pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     pDeviceContext->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer);
     pDeviceContext->VSSetConstantBuffers(1, 1, &pViewMatrixBuffer);
     pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);

     pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
     pDeviceContext->DrawIndexed(36, 0, 0);

     pDeviceContext->VSSetShader(pVertexShader2, nullptr, 0);
     pDeviceContext->DrawIndexed(36, 0, 0);

     sky.Render();

     trans.Render();

     HRESULT result = pSwapChain->Present(0, 0);
     return SUCCEEDED(result);
}

bool Renderer::Update()
{
     static float t = 0.0f;
     static ULONGLONG timeStart = 0;
     ULONGLONG timeCur = GetTickCount64();
     if (timeStart == 0) 
     {
          timeStart = timeCur;
     }
     t = (timeCur - timeStart) / 1000.0f;

     WorldMatrixBuffer worldMatrixBuffer;
     t = 0;
     worldMatrixBuffer.worldMatrix = DirectX::XMMatrixRotationY(t);

     pDeviceContext->UpdateSubresource(pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

     DirectX::XMMATRIX view = pCamera->GetViewMatrix();

     DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, width / (FLOAT)height, 100.0f, 0.1f);

     D3D11_MAPPED_SUBRESOURCE subresource;
     HRESULT result = pDeviceContext->Map(pViewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
     if (SUCCEEDED(result)) 
     {
          ViewMatrixBuffer& sceneBuffer = *reinterpret_cast<ViewMatrixBuffer*>(subresource.pData);
          sceneBuffer.viewProjMatrix = XMMatrixMultiply(view, projection);
          pDeviceContext->Unmap(pViewMatrixBuffer, 0);
     }
     return SUCCEEDED(result) && sky.Update(view, projection, pCamera->GetPosition());
}

HRESULT Renderer::SetupBackBuffer() {
     ID3D11Texture2D* pBackBuffer = NULL;
     HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
     if (SUCCEEDED(hr)) {
          hr = pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pBackBufferRTV);
          SAFE_RELEASE(pBackBuffer);
     }

     return hr;
}

HRESULT Renderer::CompileShaders()
{
     ID3D10Blob* vertexShaderBuffer = nullptr;
     ID3D10Blob* vertexShaderBuffer2 = nullptr;
     ID3D10Blob* pixelShaderBuffer = nullptr;
     
     int flags = 0;
#ifdef _DEBUG
     flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

     HRESULT hr = D3DCompileFromFile(L"vertex_shader.hlsl", NULL, NULL, 
          "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
     if (!SUCCEEDED(hr))
          return hr;
     
     hr = pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), 
          vertexShaderBuffer->GetBufferSize(), NULL, &pVertexShader);
     if (!SUCCEEDED(hr))
          return hr;

     hr = D3DCompileFromFile(L"vertex_shader2.hlsl", NULL, NULL,
          "main", "vs_5_0", flags, 0, &vertexShaderBuffer2, NULL);
     if (!SUCCEEDED(hr))
          return hr;

     hr = pDevice->CreateVertexShader(vertexShaderBuffer2->GetBufferPointer(),
          vertexShaderBuffer2->GetBufferSize(), NULL, &pVertexShader2);
     if (!SUCCEEDED(hr))
          return hr;
    
     hr = D3DCompileFromFile(L"pixel_shader.hlsl", NULL, NULL, 
          "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
     if (!SUCCEEDED(hr))
          return hr;
     
     hr = pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), 
          pixelShaderBuffer->GetBufferSize(), NULL, &pPixelShader);
     if (!SUCCEEDED(hr))
          return hr;

     static const D3D11_INPUT_ELEMENT_DESC InputDesc[] = {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0} };
     int numElements = sizeof(InputDesc) / sizeof(InputDesc[0]);
     hr = pDevice->CreateInputLayout(InputDesc, numElements, 
          vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pInputLayout);
     
     SAFE_RELEASE(vertexShaderBuffer);
     SAFE_RELEASE(vertexShaderBuffer2);
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

HRESULT Renderer::CreateSceneMatrixBuffer()
{
     D3D11_BUFFER_DESC desc = {};
     desc.ByteWidth = sizeof(ViewMatrixBuffer);
     desc.Usage = D3D11_USAGE_DYNAMIC;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
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

struct TextureDesc​
{
     UINT32 pitch = 0;
     UINT32 mipmapsCount = 0;
     DXGI_FORMAT fmt = DXGI_FORMAT_UNKNOWN;
     UINT32 width = 0;
     UINT32 height = 0;
     void* pData = nullptr;
};

HRESULT Renderer::CreateTexture()
{
     return DirectX::CreateDDSTextureFromFile(pDevice, L"textures/eye.dds", nullptr, &pTextureView);
}

HRESULT Renderer::CreateSampler()
{
     D3D11_SAMPLER_DESC desc = {};
     desc.Filter = D3D11_FILTER_ANISOTROPIC;
     desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
     desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
     desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
     desc.MinLOD = -D3D11_FLOAT32_MAX;
     desc.MaxLOD = D3D11_FLOAT32_MAX;
     desc.MipLODBias = 0.0f;
     desc.MaxAnisotropy = 16;
     desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
     desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 1.0f;
     return pDevice->CreateSamplerState(&desc, &pSamplerState);
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

bool Renderer::Resize(const unsigned width, const unsigned height)
{
     if (width != this->width || height != this->height) {
          SAFE_RELEASE(pBackBufferRTV);
          sky.Resize(width, height);
          HRESULT hr = pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
          if (SUCCEEDED(hr)) {
               this->width = width;
               this->height = height;

               hr = SetupBackBuffer();
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
     SAFE_RELEASE(pSamplerState);
     SAFE_RELEASE(pTextureView);
     SAFE_RELEASE(pDepthBuffer);
     SAFE_RELEASE(pDepthBufferDSV);
     SAFE_RELEASE(pDepthState);
}

Renderer::~Renderer() {
     Cleanup();
}
