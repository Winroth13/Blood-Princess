#include "../shared_resources.hlsli"

struct VertexShaderInput
{
    float3 position         : POSITION;
    float3 normal           : NORMAL;
    float2 uv0              : TEXCOORD0;
    float2 uv1              : TEXCOORD1;
};

struct VertexShaderOutput
{
    float4 position             : SV_POSITION;
    float3 worldPosition        : TEXCOORD0;
    float2 uv1                  : TEXCOORD1;
    float3 normal               : NORMAL;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // Remap UV from [0, 1] to NDC space [-1, 1]
    output.position.x = input.uv1.x * 2.0f - 1.0f;
    output.position.y = (1.0f - input.uv1.y) * 2.0f - 1.0f; // Flip Y for DX11 coordinate space
    output.position.z = 0.0f;
    output.position.w = 1.0f;

    output.normal = input.normal;
    
    // Pass world-space position to Pixel Shader for distance testing
    output.worldPosition = mul(float4(input.position, 1.0f), worldMatrix).xyz;
    output.uv1 = input.uv1;
    
    return output;
}