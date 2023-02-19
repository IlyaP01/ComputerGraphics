struct VSOutput
{
     float4 pos : SV_POSITION;
     float4 color: COLOR;
};

float4 main(VSOutput pixel) : SV_TARGET
{
     return pixel.color;
}
