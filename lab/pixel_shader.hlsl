#include "calc_color.hlsli"

Texture2D cubeTexture : register (t0);
Texture2D cubeNormalTexture : register (t1);
SamplerState cubeSampler : register(s0);
SamplerState cubeNormalSampler : register (s1);

cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
     float4 shine;
};

struct VSOutput
{
     float4 position : SV_Position;
     float4 worldPos : POSITION;
     float2 texCoord : TEXCOORD;
     float3 normal : NORMAL;
     float3 tangent : TANGENT;
};

float4 main(VSOutput input) : SV_Target0
{
     float3 color = cubeTexture.Sample(cubeSampler, input.texCoord).xyz;
     float3 finalColor = ambientColor.xyz * color;

     float3 norm = float3(0, 0, 0);
     if (lightCount.y > 0)
     {
          float3 binorm = normalize(cross(input.normal, input.tangent));
          float3 localNorm = cubeNormalTexture.Sample(cubeNormalSampler, input.texCoord).xyz * 2.0 - 1.0;
          norm = localNorm.x * normalize(input.tangent) + localNorm.y * binorm + localNorm.z * normalize(input.normal);
     }
     else
     {
          norm = input.normal;
     }

     return float4(CalculateColor(color, norm, input.worldPos.xyz, shine.x, false), 1.0);
}