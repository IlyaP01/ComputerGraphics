#include "calc_color.hlsli"

cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
     float4 color;
};

struct VSOutput
{
     float4 position : SV_Position;
     float4 worldPos : POSITION;
};

float4 main(VSOutput input) : SV_Target0
{
     return float4(CalculateColor(color.xyz, float3(1, 0, 0), input.worldPos.xyz, 0.0, true), color.w);
}