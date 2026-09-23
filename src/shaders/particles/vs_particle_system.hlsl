#include "../shared_resources.hlsli"

struct VertexShaderOutput
{
    float3 worldPosition : WORLD_POSITION;
    float3 tint : TINT;
    float scale : SCALE;
    float lifetime : LIFETIME;
};

StructuredBuffer<Particle> particles : register(t0);

VertexShaderOutput main(uint vertexID : SV_VertexID)
{
    VertexShaderOutput output;

    float4 worldPos = mul(float4(particles[vertexID].position, 1.0f), worldMatrix);

    output.worldPosition = worldPos.xyz;
    output.scale = particles[vertexID].scale;
    output.tint = particles[vertexID].tint;
    output.lifetime = particles[vertexID].lifeTime;

    return output;
}