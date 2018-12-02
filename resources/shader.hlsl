uniform float4x4 ProjectionMatrix;
struct VS_INPUT
{
    float2 pos : POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};
struct PS_INPUT
{
    float4 pos : POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};
PS_INPUT vert(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

sampler2D sampler0;
float4 frag(PS_INPUT input) : COLOR0
{
    float4 out_col = tex2D(sampler0, input.uv) *
#if D3D11
        input.col;
#else
        input.col.bgra;
#endif

    return out_col;
}

