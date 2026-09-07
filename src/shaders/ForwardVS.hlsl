cbuffer MatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewProjMatrix;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4 worldPosition = mul(float4(input.position, 1.0f), worldMatrix);
    output.clipPosition = mul(worldPosition, viewProjMatrix);
    output.uv = input.uv;

    return output;
}
