cbuffer ub_global : register(b0)
{
   float4x4 uc_modelViewProjection;   
};

struct VS_PS_DATA
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

#ifdef VERTEX_SHADER

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

VS_PS_DATA main(VS_INPUT input)
{
    VS_PS_DATA output;    
    output.position = mul(uc_modelViewProjection, float4(input.position,1));
    output.color    = input.color;    
    return output;
}

#endif

#ifdef PIXEL_SHADER

float4 main(VS_PS_DATA input) : SV_TARGET
{
    return input.color;
}

#endif