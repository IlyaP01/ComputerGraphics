cbuffer WorldMatrixBuffer : register (b0)
{
     float4x4 worldMatrix;
}

cbuffer ViewMatrixBuffer : register (b1)
{
     float4x4 viewProjectionMatrix;
};

struct VSInput
{
     float3 position : POSITION;
     float2 uv: TEXCOORD;
};

struct VSOutput {
     float4 position : SV_POSITION;
     float2 uv: TEXCOORD;
};
     
VSOutput main(VSInput input)
{
     VSOutput output;
     output.position = mul(viewProjectionMatrix, mul(worldMatrix, float4(input.position, 1.0f)));
     output.uv = input.uv;

     return output;
}
