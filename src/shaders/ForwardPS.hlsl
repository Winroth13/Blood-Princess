Texture2D shaderTexture : register(t0);
SamplerState samplerState : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return shaderTexture.Sample(samplerState, input.uv);
}
