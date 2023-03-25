#pragma once

#include "Camera.h"
#include "Sky.h"
#include "Transparent.h"
#include "Lights.h"

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
          DirectX::XMFLOAT3 pos;
          DirectX::XMFLOAT2 uv;
          DirectX::XMFLOAT3 normal;
          DirectX::XMFLOAT3 tangent;
     };
     
     const Vertex vertices[24] = {
          Vertex{{-0.5, -0.5, 0.5}, {0, 1}, {0, -1, 0}, {1, 0, 0}},
          Vertex{{0.5, -0.5, 0.5}, {1, 1}, {0, -1, 0}, {1, 0, 0}},
          Vertex{{0.5, -0.5, -0.5}, {1, 0}, {0, -1, 0}, {1, 0, 0}},
          Vertex{{-0.5, -0.5, -0.5}, {0, 0}, {0, -1, 0}, {1, 0, 0}},

          Vertex{{-0.5, 0.5, -0.5}, {0, 1}, {0, 1, 0}, {1, 0, 0}},
          Vertex{{0.5, 0.5, -0.5}, {1, 1}, {0, 1, 0}, {1, 0, 0}},
          Vertex{{0.5, 0.5, 0.5}, {1, 0}, {0, 1, 0}, {1, 0, 0}},
          Vertex{{-0.5, 0.5, 0.5}, {0, 0}, {0, 1, 0}, {1, 0, 0}},

          Vertex{{0.5, -0.5, -0.5}, {0, 1}, {1, 0, 0}, {0, 0, 1}},
          Vertex{{0.5, -0.5, 0.5}, {1, 1}, {1, 0, 0}, {0, 0, 1}},
          Vertex{{0.5, 0.5, 0.5}, {1, 0}, {1, 0, 0}, {0, 0, 1}},
          Vertex{{0.5, 0.5, -0.5}, {0, 0}, {1, 0, 0}, {0, 0, 1}},

          Vertex{{-0.5, -0.5, 0.5}, {0, 1}, {-1, 0, 0}, {0, 0, -1}},
          Vertex{{-0.5, -0.5, -0.5}, {1, 1}, {-1, 0, 0}, {0, 0, -1}},
          Vertex{{-0.5, 0.5, -0.5}, {1, 0}, {-1, 0, 0}, {0, 0, -1}},
          Vertex{{-0.5, 0.5, 0.5}, {0, 0}, {-1, 0, 0}, {0, 0, -1}},

          Vertex{{0.5, -0.5, 0.5}, {0, 1}, {0, 0, 1}, {-1, 0, 0}},
          Vertex{{-0.5, -0.5, 0.5}, {1, 1}, {0, 0, 1}, {-1, 0, 0}},
          Vertex{{-0.5, 0.5, 0.5}, {1, 0}, {0, 0, 1}, {-1, 0, 0}},
          Vertex{{0.5, 0.5, 0.5}, {0, 0}, {0, 0, 1}, {-1, 0, 0}},

          Vertex{{-0.5, -0.5, -0.5}, {0, 1}, {0, 0, -1}, {1, 0, 0}},
          Vertex{{0.5, -0.5, -0.5}, {1, 1}, {0, 0, -1}, {1, 0, 0}},
          Vertex{{0.5, 0.5, -0.5}, {1, 0}, {0, 0, -1}, {1, 0, 0}},
          Vertex{{-0.5, 0.5, -0.5}, {0, 0}, {0, 0, -1}, {1, 0, 0}}
     };
     
     const USHORT indices[36] = {
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
     };

     static constexpr const DirectX::XMFLOAT4 ambientColor_{ 0.5f, 0.5f, 0.5f, 1.0f };

     Renderer() = default;
     HRESULT SetupBackBuffer();
     HRESULT CompileShaders();
     HRESULT CreateVertexBuffer();
     HRESULT CreateIndexBuffer();
     HRESULT CreateWorldMatrixBuffer();
     HRESULT CreateSceneMatrixBuffer();
     HRESULT CreateRasterizerState();
     HRESULT CreateTextures();
     HRESULT CreateSamplers();
     HRESULT CreateDepthBuffer();
     HRESULT CreateDepthState();

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
     ID3D11Buffer* pWorldMatrixBuffer2 = nullptr;
     ID3D11Buffer* pViewMatrixBuffer = nullptr;
     ID3D11RasterizerState* pRasterizerState = nullptr;

     ID3D11ShaderResourceView* pCubeTexture = nullptr;
     ID3D11ShaderResourceView* pCubeNormalMap = nullptr;

     ID3D11SamplerState* pCubeTextureSampler = nullptr;
     ID3D11SamplerState* pCubeNormalsSampler = nullptr;

     ID3D11Texture2D* pDepthBuffer = nullptr;
     ID3D11DepthStencilView* pDepthBufferDSV = nullptr;
     ID3D11DepthStencilState* pDepthState = nullptr;

     unsigned width = 0;
     unsigned height = 0;

     Sky sky;
     Transparent trans;
     Lights lights;
};
