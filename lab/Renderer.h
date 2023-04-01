#pragma once

#include "Camera.h"
#include "Sky.h"
#include "Transparent.h"
#include "Lights.h"
#include "Frustum.h"
#include "PostProc.h"

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
     struct SceneBuffer
     {
          DirectX::XMMATRIX viewProjMatrix;
          DirectX::XMFLOAT4 cameraPosition;
          int lightCount[4];
          DirectX::XMFLOAT4 lightPositions[maxLightNumber];
          DirectX::XMFLOAT4 lightColors[maxLightNumber];
          DirectX::XMFLOAT4 ambientColor;
     };

     struct WorldMatrixBuffer
     {
          DirectX::XMMATRIX worldMatrix;
          DirectX::XMFLOAT4 shine;
     };

     struct Vertex 
     {
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

     const DirectX::XMFLOAT4 AABB[2] = {
          {-0.5, -0.5, -0.5, 1.0},
          {0.5,  0.5, 0.5, 1.0}
     };

     static constexpr const DirectX::XMFLOAT4 ambientColor_{ 0.8f, 0.8f, 0.8f, 1.0f };
     static constexpr const size_t maxInst = 20;

     Renderer() = default;
     HRESULT SetupBackBuffer();
     HRESULT CompileShaders();
     HRESULT CreateVertexBuffer();
     HRESULT CreateIndexBuffer();
     HRESULT CreateWorldMatrixBuffer();
     HRESULT CreateWorldBufferInstVis();
     HRESULT CreateSceneMatrixBuffer();
     HRESULT CreateRasterizerState();
     HRESULT CreateTextures();
     HRESULT CreateSamplers();
     HRESULT CreateDepthBuffer();
     HRESULT CreateDepthState();
     HRESULT InitRenderTargetTexture();

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
     ID3D11Buffer* pWorldBufferInstVis = nullptr;
     ID3D11Buffer* pViewMatrixBuffer = nullptr;
     ID3D11RasterizerState* pRasterizerState = nullptr;

     ID3D11ShaderResourceView* pTextureView = nullptr;
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
     std::vector<WorldMatrixBuffer> worldMatricies;
     Frustum frustum;
     PostProc postProc;
     std::vector<XMINT4> ids;

     ID3D11Texture2D* pRenderTargetTexture = nullptr;
     ID3D11RenderTargetView* pRenderTargetView = nullptr;
     ID3D11ShaderResourceView* pShaderResourceViewRenderResult = nullptr;
     D3D11_VIEWPORT viewport_;
};
