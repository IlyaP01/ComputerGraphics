#include "lights.h"
#include <algorithm>

bool Lights::Add(const LightInfo& info)
{
     if (lights.size() >= maxLightNumber)
          return false;
     lights.push_back(info);
     return true;
}

std::size_t Lights::GetNumber()
{
     return lights.size();
}

std::vector<DirectX::XMFLOAT4> Lights::GetPositions(std::size_t milliseconds)
{
     std::vector<DirectX::XMFLOAT4> res;
     res.reserve(lights.size());
     std::for_each(
          lights.begin(),
          lights.end(),
          [milliseconds, &res](auto& lightInfo) {res.push_back(lightInfo.positionGetter(milliseconds)); });
     return res;
}

std::vector<DirectX::XMFLOAT4> Lights::GetColors(std::size_t milliseconds)
{
     std::vector<DirectX::XMFLOAT4> res;
     res.reserve(lights.size());
     std::for_each(
          lights.begin(),
          lights.end(),
          [milliseconds, &res](auto& lightInfo) {res.push_back(lightInfo.colorGetter(milliseconds)); });
     return res;
}