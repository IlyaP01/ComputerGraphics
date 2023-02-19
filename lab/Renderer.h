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
     struct Vertex {
          float x, y, z;
          COLORREF color;
     };

     const Vertex vertices[3] = {
        {-0.5f, -0.5f, 0.0f, RGB(255, 0, 0)},
        { 0.5f, -0.5f, 0.0f, RGB(0, 255, 0)},
        { 0.0f,  0.5f, 0.0f, RGB(0, 0, 255)}
     };
     
     const USHORT indices[3] = {0, 2, 1};

     Renderer() = default;
     ~Renderer();
     void Cleanup();
     HRESULT SetupBackBuffer();
     HRESULT CompileShaders();
     HRESULT CreateVertexBuffer();
     HRESULT CreateIndexBuffer();

     ID3D11Device* pDevice = nullptr;
     ID3D11DeviceContext* pDeviceContext = nullptr;
 
     IDXGISwapChain* pSwapChain = nullptr;
     ID3D11RenderTargetView* pBackBufferRTV = nullptr;

     ID3D11VertexShader* pVertexShader = nullptr;
     ID3D11PixelShader* pPixelShader = nullptr;
     ID3D11InputLayout* pInputLayout = nullptr;

     ID3D11Buffer* pVertexBuffer = nullptr;
     ID3D11Buffer* pIndexBuffer = nullptr;

     unsigned width = 0;
     unsigned height = 0;
};
