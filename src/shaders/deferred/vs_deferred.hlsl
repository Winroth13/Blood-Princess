#include "../shared_resources.hlsli"
#include "deferred_vertex.hlsli"

// Input
struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv0      : TEXCOORD0;
    float2 uv1      : TEXCOORD1;
    float4 tangent  : TANGENT;
};

VertexShaderOutput main(VertexShaderInput input)
{
    return TransformVertex(input.position, input.normal, input.uv0, input.uv1, input.tangent);
}