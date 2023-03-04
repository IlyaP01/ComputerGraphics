#include "Camera.h"

#include <algorithm>

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 focus, float phi, float theta, float r) 
     : focus(focus)
     , phi(phi)
     , theta(theta)
     , r(r)
{
     CalcMatrix();
}

const DirectX::XMMATRIX& Camera::GetViewMatrix() const
{
     return viewMatrix;
}

void Camera::MoveCamera(float dPhi, float dTheta, float dR)
{
     phi -= dPhi;
     theta += dTheta;
     theta = std::min(std::max(theta, -XM_PIDIV2), XM_PIDIV2);
     r += dR;
     if (r < 1.0f) {
          r = 1.0f;
     }
     CalcMatrix();
}

XMFLOAT3 Camera::GetPosition() const
{
     XMFLOAT3 eye = XMFLOAT3(cosf(theta) * cosf(phi), sinf(theta), cosf(theta) * sinf(phi));
     eye.x = eye.x * r + focus.x;
     eye.y = eye.y * r + focus.y;
     eye.z = eye.z * r + focus.z;

     return eye;
}

inline void Camera::CalcMatrix()
{
     XMFLOAT3 eye = XMFLOAT3(cosf(theta) * cosf(phi), sinf(theta), cosf(theta) * sinf(phi));
     eye.x = eye.x * r + focus.x;
     eye.y = eye.y * r + focus.y;
     eye.z = eye.z * r + focus.z;
     float upTheta = theta + XM_PIDIV2;
     XMFLOAT3 up = XMFLOAT3(cosf(upTheta) * cosf(phi), sinf(upTheta), cosf(upTheta) * sinf(phi));

     viewMatrix = DirectX::XMMatrixLookAtLH(
          DirectX::XMVectorSet(eye.x, eye.y, eye.z, 0.0f),
          DirectX::XMVectorSet(focus.x, focus.y, focus.z, 0.0f),
          DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f)
     );
}
