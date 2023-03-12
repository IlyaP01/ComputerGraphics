cbuffer WorldMatrixBuffer : register (b0) 
{
	float4x4 worldMatrix;
};

cbuffer SceneMatrixBuffer : register (b1) 
{
	float4x4 viewProjMatrix;
};

struct VSInput 
{
	float3 position : POSITION;
};

struct VSOutput 
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VSOutput main(VSInput input) 
{
	VSOutput output;

	output.position = mul(viewProjMatrix, mul(worldMatrix, float4(input.position, 1.0f)));
	output.color = float4(0.0, 0.0, 1.0, 0.5);

	return output;
}