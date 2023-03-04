#pragma once

#include <d3d11.h>
#include <directxmath.h>

class Sky
{
public:
     bool Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, int width, int height);
     void Render();
     bool Update(DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix, DirectX::XMFLOAT3 cameraPos);
     void Resize(int width, int height);
     void Cleanup();
     ~Sky();
private:
     struct Vertex {
          float x, y, z;
     };

     HRESULT CreateSphere(int LatLines, int LongLines);
     HRESULT CompileShaders();
     HRESULT CreateTexture();
     HRESULT CreateSampler();
     HRESULT CreateRasterizerState();
     HRESULT CreateWorldMatrixBuffer();
     HRESULT CreateSceneMatrixBuffer();

     ID3D11Device* pDevice;
     ID3D11DeviceContext* pDeviceContext;
     ID3D11VertexShader* pVertexShader = nullptr;
     ID3D11PixelShader* pPixelShader = nullptr;
     ID3D11InputLayout* pInputLayout = nullptr;

     ID3D11Buffer* pVertexBuffer = nullptr;
     ID3D11Buffer* pIndexBuffer = nullptr;
     
     ID3D11Buffer* pWorldMatrixBuffer = nullptr;
     ID3D11Buffer* pViewMatrixBuffer = nullptr;
     
     ID3D11RasterizerState* pRasterizerState = nullptr;
     ID3D11SamplerState* pSamplerState = nullptr;


     ID3D11ShaderResourceView* pTextureView;

     int numSphereVertices = 0;
     int numSphereFaces = 0;
     float radius = 1.0f;

     unsigned width = 0;
     unsigned height = 0;
};

