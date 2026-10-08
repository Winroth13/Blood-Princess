#include "deferred_vertex.hlsli"
#include "../skinning.hlsli"

// Input
struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float4 tangent : TANGENT;
    
    uint4 joints : JOINTS;
    float4 weights : WEIGHTS;
};

VertexShaderOutput main(VertexShaderInput input)
{
    float4x4 skinMatrix = GetSkinMatrix(input.joints, input.weights);
    
    float3 position = mul(float4(input.position, 1.0f), skinMatrix).xyz;
    float3 normal = mul(input.normal, (float3x3) skinMatrix);
    float3 tangent = mul(input.tangent.xyz, (float3x3) skinMatrix);
    
    return TransformVertex(position, normal, input.uv0, input.uv1, float4(tangent, input.tangent.w));
}