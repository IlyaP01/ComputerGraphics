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
     float4 color : COLOR;
};

struct VSOutput {
     float4 position : SV_POSITION;
     float4 color : COLOR;
};

VSOutput main(VSInput input)
{
     VSOutput output;
     output.position = mul(viewProjectionMatrix, mul(worldMatrix, float4(input.position, 1.0f)));
     output.color = input.color;

     return output;
}
