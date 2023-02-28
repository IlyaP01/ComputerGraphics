#pragma once

#include <directxmath.h>

class Camera
{
public:
     Camera(
          DirectX::XMFLOAT3 focus = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
          float phi = DirectX::XM_PIDIV4,
          float theta = -DirectX::XM_PIDIV4,
          float r = 4.0f);
     const DirectX::XMMATRIX& GetViewMatrix() const;
     void MoveCamera(float dPhi, float dTheta, float dR);
private:
     DirectX::XMMATRIX viewMatrix;
     DirectX::XMFLOAT3 focus;
     float phi;
     float theta;
     float r;

     inline void CalcMatrix();
};

