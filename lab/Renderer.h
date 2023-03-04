#pragma once

#include "Camera.h"
#include "Sky.h"

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
     void Cleanup();

     Renderer(const Renderer&) = delete;
     Renderer& operator=(const Renderer&) = delete;

     ~Renderer();
private:
     struct Vertex {
          float x, y, z;
          float u, v;
     };
     
     const Vertex vertices[24] = {
          // Bottom face
           {-0.5, -0.5,  0.5, 0, 1},
           { 0.5, -0.5,  0.5, 1, 1},
           { 0.5, -0.5, -0.5, 1, 0},
           {-0.5, -0.5, -0.5, 0, 0},
           // Top face
           {-0.5,  0.5, -0.5, 0, 1},
           { 0.5,  0.5, -0.5, 1, 1},
           { 0.5,  0.5,  0.5, 1, 0},
           {-0.5,  0.5,  0.5, 0, 0},
           // Front face
           { 0.5, -0.5, -0.5, 0, 1},
           { 0.5, -0.5,  0.5, 1, 1},
           { 0.5,  0.5,  0.5, 1, 0},
           { 0.5,  0.5, -0.5, 0, 0},
           // Back face
           {-0.5, -0.5,  0.5, 0, 1},
           {-0.5, -0.5, -0.5, 1, 1},
           {-0.5,  0.5, -0.5, 1, 0},
           {-0.5,  0.5,  0.5, 0, 0},
           // Left face
           { 0.5, -0.5,  0.5, 0, 1},
           {-0.5, -0.5,  0.5, 1, 1},
           {-0.5,  0.5,  0.5, 1, 0},
           { 0.5,  0.5,  0.5, 0, 0},
           // Right face
           {-0.5, -0.5, -0.5, 0, 1},
           { 0.5, -0.5, -0.5, 1, 1},
           { 0.5,  0.5, -0.5, 1, 0},
           {-0.5,  0.5, -0.5, 0, 0}
     };
     
     const USHORT indices[36] = {
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
     };

     Renderer() = default;
     HRESULT SetupBackBuffer();
     HRESULT CompileShaders();
     HRESULT CreateVertexBuffer();
     HRESULT CreateIndexBuffer();
     HRESULT CreateWorldMatrixBuffer();
     HRESULT CreateSceneMatrixBuffer();
     HRESULT CreateRasterizerState();
     HRESULT CreateTexture();
     HRESULT CreateSampler();

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
     ID3D11SamplerState* pSamplerState = nullptr;

     ID3D11ShaderResourceView* pTextureView = nullptr;

     unsigned width = 0;
     unsigned height = 0;

     Sky sky;
};
