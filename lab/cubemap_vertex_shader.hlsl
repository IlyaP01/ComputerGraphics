cbuffer WorldMatrixBuffer : register (b0)
{
     float4x4 worldMatrix;
     float4 size;
};

cbuffer SceneMatrixBuffer : register (b1)
{
     float4x4 viewProjMatrix;
     float4 cameraPos;
};

struct VSInput
{
     float3 position : POSITION;
};

struct VSOutput {
     float4 position : SV_POSITION;
     float3 localPos : POSITION1;
};

VSOutput main(VSInput input) {
     VSOutput output;

     float3 pos = cameraPos.xyz + input.position * size.x;
     output.position = mul(viewProjMatrix, mul(worldMatrix, float4(pos, 1.0f)));
     output.position.z = 0;
     output.localPos = input.position;

     return output;
}
