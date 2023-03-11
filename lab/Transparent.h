#pragma once

#include <d3d11.h>
#include <directxmath.h>

class Transparent
{
public:
     bool Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, int width, int height);
     void Render();
     void Cleanup();
     ~Transparent();
private:
     struct Vertex {
          float x, y, z;
     };

     const Vertex vertices[4] = {
          {0.6, -1, -1},
          {0.6,  1, -1},
          {0.6,  1,  1},
          {0.6, -1,  1}
     };

     const USHORT indices[6] = {
        0, 2, 1, 0, 3, 2
     };

     HRESULT CreateVertexBuffer();
     HRESULT CreateIndexBuffer();
     HRESULT CompileShaders();
     HRESULT CreateRasterizerState();
     HRESULT CreateBlendState();
     HRESULT CreateDepthState();

     ID3D11Device* pDevice = nullptr;
     ID3D11DeviceContext* pDeviceContext = nullptr;

     ID3D11Buffer* pVertexBuffer = nullptr;
     ID3D11Buffer* pIndexBuffer = nullptr;

     ID3D11VertexShader* pVertexShader = nullptr;
     ID3D11VertexShader* pVertexShader2 = nullptr;
     ID3D11PixelShader* pPixelShader = nullptr;
     
     ID3D11RasterizerState* pRasterizerState = nullptr;
     ID3D11InputLayout* pInputLayout = nullptr;

     ID3D11DepthStencilState* pDepthState = nullptr;
     ID3D11BlendState* pBlendState = nullptr;
};

