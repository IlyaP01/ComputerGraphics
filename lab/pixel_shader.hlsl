#include "calc_color.hlsli"

Texture2DArray cubeTexture : register (t0);
Texture2D cubeNormalTexture : register (t1);
SamplerState cubeSampler : register(s0);
SamplerState cubeNormalSampler : register (s1);

struct WorldBuffer
{
     float4x4 world;
     float4 shine;
};

cbuffer WorldBufferInst : register (b0)
{
     WorldBuffer worldBuffer[20];
};

struct VSOutput
{
     float4 position : SV_Position;
     float4 worldPos : POSITION;
     float2 texCoord : TEXCOORD;
     float3 normal : NORMAL;
     float3 tangent : TANGENT;
     nointerpolation uint instanceId : INST_ID;
};

float4 main(VSOutput input) : SV_Target0
{
     unsigned int idx = input.instanceId;
     float3 color = cubeTexture.Sample(cubeSampler, float3(input.texCoord, worldBuffer[idx].shine.z)).xyz;
     float3 finalColor = ambientColor.xyz * color;
     if (idx % 2 == 1)
     {
          return float4(finalColor, 1.0);
     }

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

     return float4(CalculateColor(color, norm, input.worldPos.xyz, worldBuffer[idx].shine.x, false), 1.0);
}