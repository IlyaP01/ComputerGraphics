#include "scene_buffer.hlsli"

struct WorldBuffer
{
     float4x4 world;
     float4 shine;
};

cbuffer WorldBufferInst : register (b0)
{
     WorldBuffer worldBuffer[100];
};

cbuffer WorldBufferInstVis : register (b2)
{
     uint4 ids[100];
}

struct VSInput
{
     float3 position : POSITION;
     float2 texCoord : TEXCOORD;
     float3 normal : NORMAL;
     float3 tangent : TANGENT;
     uint instanceId : SV_InstanceID;
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

VSOutput main(VSInput input)
{
     VSOutput output;

     unsigned int idx = ids[input.instanceId].x;

     output.worldPos = mul(worldBuffer[idx].world, float4(input.position, 1.0f));
     output.position = mul(viewProj, output.worldPos);
     output.texCoord = input.texCoord;
     output.normal = mul(worldBuffer[idx].world, float4(input.normal, 1.0f)).xyz;
     output.tangent = mul(worldBuffer[idx].world, float4(input.tangent, 1.0f)).xyz;
     output.instanceId = idx; 

     return output;
}
