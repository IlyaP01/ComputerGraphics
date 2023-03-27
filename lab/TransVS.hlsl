#include "scene_buffer.hlsli"

cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
     float4 color;
};

struct VSInput
{
     float3 position : POSITION;
};

struct VSOutput
{
     float4 position : SV_Position;
     float4 worldPos : POSITION;
};

VSOutput main(VSInput input)
{
     VSOutput output;
     output.worldPos = mul(world, float4(input.position, 1.0f));
     output.position = mul(viewProj, output.worldPos);

     return output;
}