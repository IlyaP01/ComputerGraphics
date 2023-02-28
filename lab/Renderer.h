#pragma once

#include "Camera.h"

#include <d3d11.h>
#include <dxgi.h>

#include <memory>

class Renderer 
{
public:
     static Renderer& GetInstance();
     bool Init(HWND hWnd, std::shared_ptr<const Camera> pCamera);
     bool Render();
     bool Update();
     bool Resize(const unsigned width, const unsigned height);

     Renderer(const Renderer&) = delete;
     Renderer& operator=(const Renderer&) = delete;
private:
     struct Vertex {
          float x, y, z;
          COLORREF color;
     };/*
     const Vertex vertices[3] = {
        {-0.5f, -0.5f, 0.0f, RGB(255, 0, 0)},
        { 0.5f, -0.5f, 0.0f, RGB(0, 255, 0)},
        { 0.0f,  0.5f, 0.0f, RGB(0, 0, 255)}
     };
     
     const USHORT indices[3] = { 0, 2, 1 };*/
     const Vertex vertices[8] = {
        { -1.0f, 1.0f, -1.0f, RGB(0, 0, 255) },
        { 1.0f, 1.0f, -1.0f, RGB(0, 255, 0) },
        { 1.0f, 1.0f, 1.0f, RGB(0, 255, 255) },
        { -1.0f, 1.0f, 1.0f, RGB(255, 0, 0) },
        { -1.0f, -1.0f, -1.0f, RGB(255, 0, 255) },
        { 1.0f, -1.0f, -1.0f, RGB(255, 255, 0) },
        { 1.0f, -1.0f, 1.0f, RGB(255, 255, 255) },
        { -1.0f, -1.0f, 1.0f, RGB(0, 0, 0) }
     };
     
     const USHORT indices[36] = {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
     };

     Renderer() = default;
     ~Renderer();
     void Cleanup();
     HRESULT SetupBackBuffer();
     HRESULT CompileShaders();
     HRESULT CreateVertexBuffer();
     HRESULT CreateIndexBuffer();
     HRESULT CreateWorldMatrixBuffer();
     HRESULT CreatesceneMatrixBuffer();
     HRESULT CreateRasterizerState();

     std::shared_ptr<const Camera> pCamera = nullptr;

     ID3D11Device* pDevice = nullptr;
     ID3D11DeviceContext* pDeviceContext = nullptr;
 
     IDXGISwapChain* pSwapChain = nullptr;
     ID3D11RenderTargetView* pBackBufferRTV = nullptr;

     ID3D11VertexShader* pVertexShader = nullptr;
     ID3D11PixelShader* pPixelShader = nullptr;
     ID3D11InputLayout* pInputLayout = nullptr;

     ID3D11Buffer* pVertexBuffer = nullptr;
     ID3D11Buffer* pIndexBuffer = nullptr;

     ID3D11Buffer* pWorldMatrixBuffer = nullptr;
     ID3D11Buffer* pViewMatrixBuffer = nullptr;
     ID3D11RasterizerState* pRasterizerState = nullptr;

     unsigned width = 0;
     unsigned height = 0;
};
