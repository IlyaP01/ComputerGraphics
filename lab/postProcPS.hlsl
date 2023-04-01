Texture2D sourceTexture : register(t0);

SamplerState Sampler : register(s0);

struct PS_INPUT
{
     float4 pos : SV_POSITION;
     float2 tex : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET{
    float3 color = sourceTexture.Sample(Sampler, input.tex).xyz;
    float gray = (color.x + color.y + color.z) / 3;
    color = float3(gray, gray, gray);
    return float4(color, 1.0);
}