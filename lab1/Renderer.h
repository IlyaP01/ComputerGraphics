#pragma once

#include <d3d11.h>
#include <dxgi.h>

class Renderer 
{
public:
     static Renderer& GetInstance();
     bool Init(HWND hWnd);
     bool Render();
     bool Resize(const unsigned width, const unsigned height);

     Renderer(const Renderer&) = delete;
     Renderer& operator=(const Renderer&) = delete;
private:
     Renderer() = default;
     ~Renderer();
     void Cleanup();
     ID3D11Device* pDevice = nullptr;
     ID3D11DeviceContext* pDeviceContext = nullptr;
     IDXGISwapChain* pSwapChain = nullptr;
     ID3D11RenderTargetView* pBackBufferRTV = nullptr;

     unsigned width = 0;
     unsigned height = 0;
};
