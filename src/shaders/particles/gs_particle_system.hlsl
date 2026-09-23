#include "../shared_resources.hlsli"

struct VertexShaderOutput
{
    float3 worldPosition : WORLD_POSITION;
    float3 tint : TINT;
    float scale : SCALE;
    float lifetime : LIFETIME;
};

struct GeometryShaderOutput
{
    float4 clipPosition : SV_POSITION;
    float3 tint : TINT;
    float lifetime : LIFETIME;
    float2 uv : UV;
};

[maxvertexcount(6)]
void main(
	point VertexShaderOutput input[1],
	inout TriangleStream<GeometryShaderOutput> output
)
{
    float4 viewPos = mul(float4(input[0].worldPosition, 1.0f), viewMatrix);
    float halfScale = input[0].scale * 0.5f;

    float4 corners[4];
    corners[0] = viewPos + float4(-halfScale, halfScale, 0.0f, 0.0f); // Top-Left
    corners[1] = viewPos + float4(halfScale, halfScale, 0.0f, 0.0f); // Top-Right
    corners[2] = viewPos + float4(-halfScale, -halfScale, 0.0f, 0.0f); // Bottom-Left
    corners[3] = viewPos + float4(halfScale, -halfScale, 0.0f, 0.0f); // Bottom-Right

    for (int i = 0; i < 4; i++)
    {
        corners[i] = mul(corners[i], projectionMatrix);
    }
    
    GeometryShaderOutput toAppend;
    toAppend.tint = input[0].tint;
    toAppend.lifetime = input[0].lifetime;
    
    // Top-left
    toAppend.clipPosition = corners[0];
    toAppend.uv = float2(0.0f, 0.0f);
    output.Append(toAppend);
    
    // Bottom-right
    toAppend.clipPosition = corners[3];
    toAppend.uv = float2(1.0f, 1.0f);
    output.Append(toAppend);
    
    // Bottom-left
    toAppend.clipPosition = corners[2];
    toAppend.uv = float2(0.0f, 1.0f);
    output.Append(toAppend);

    output.RestartStrip();
    
    // Top-left
    toAppend.clipPosition = corners[0];
    toAppend.uv = float2(0.0f, 0.0f);
    output.Append(toAppend);
    
    // Top-right
    toAppend.clipPosition = corners[1];
    toAppend.uv = float2(1.0f, 0.0f);
    output.Append(toAppend);
    
    // Bottom-right
    toAppend.clipPosition = corners[3];
    toAppend.uv = float2(1.0f, 1.0f);
    output.Append(toAppend);
}