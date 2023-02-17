#include "Renderer.h"

#define SAFE_RELEASE(DXResource) do { if ((DXResource) != NULL) { (DXResource)->Release(); } } while (false);

Renderer& Renderer::GetInstance()
{
     static Renderer instance;
     return instance;
}

bool Renderer::Init(const HWND hWnd)
{
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

     return true;
}

bool Renderer::Render()
{
     pDeviceContext->ClearState();
     ID3D11RenderTargetView* views[] = { pBackBufferRTV };
     pDeviceContext->OMSetRenderTargets(1, views, NULL);

     static const FLOAT backColor[4] = { 0.5f, 0.5f, 0.7f, 1.0f };
     pDeviceContext->ClearRenderTargetView(pBackBufferRTV, backColor);

     HRESULT result = pSwapChain->Present(0, 0);
     if (!SUCCEEDED(result))
          return false;

     return true;
}

bool Renderer::Resize(const unsigned width, const unsigned height)
{
     if (NULL == pSwapChain)
          return false;

     pDeviceContext->OMSetRenderTargets(0, 0, 0);
     pBackBufferRTV->Release();

     HRESULT result = pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
     if (!SUCCEEDED(result))
          return false;

     ID3D11Texture2D* pBuffer;
     result = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBuffer));
     if (!SUCCEEDED(result))
          return false;

     result = pDevice->CreateRenderTargetView(pBuffer, NULL, &pBackBufferRTV);
     pBuffer->Release();
     if (!SUCCEEDED(result))
          return false;

     pDeviceContext->OMSetRenderTargets(1, &pBackBufferRTV, NULL);

     D3D11_VIEWPORT vp;
     vp.Width = (FLOAT)width;
     vp.Height = (FLOAT)height;
     vp.MinDepth = 0.0f;
     vp.MaxDepth = 1.0f;
     vp.TopLeftX = 0;
     vp.TopLeftY = 0;
     pDeviceContext->RSSetViewports(1, &vp);

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
}

Renderer::~Renderer() {
     Cleanup();
}
