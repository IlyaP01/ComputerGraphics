#pragma once

#include <directxmath.h>
#include <stdint.h>
#include <functional>
#include <vector>

static const constexpr std::size_t maxLightNumber = 10;

struct LightInfo
{
     std::function<DirectX::XMFLOAT4(std::size_t milliseconds)> positionGetter;
     std::function<DirectX::XMFLOAT4(std::size_t milliseconds)> colorGetter;
};

class Lights
{
public:
     bool Add(const LightInfo& info);
     std::size_t GetNumber();
     std::vector<DirectX::XMFLOAT4> GetPositions(std::size_t milliseconds);
     std::vector<DirectX::XMFLOAT4> GetColors(std::size_t milliseconds);

private:
     std::vector<LightInfo> lights;
};