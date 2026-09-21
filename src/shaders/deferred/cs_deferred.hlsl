#include "../shared_resources.hlsli"

RWTexture2D<float4> backBufferUAV;

Texture2D<float4> albedoMetallicTexture : register(t0);
Texture2D<float4> normalRoughnessTexture : register(t1);
Texture2D<float4> depthStencilTexture : register(t2);

StructuredBuffer<DirectionalLight> directionalLights : register(t3);
StructuredBuffer<SpotLight> spotLights : register(t4);
StructuredBuffer<PointLight> pointLights : register(t5);

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    float2 ndc = float2(2.0f * uv.x - 1.0f, -2.0f * uv.y + 1.0f);
    float4 positionClip = float4(ndc, depth, 1.0f);
    float4 positionWorld = mul(positionClip, invViewProjectionMatrix);
    return positionWorld.xyz / positionWorld.w;
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    backBufferUAV.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
        return;
            
    int2 pixel = int2(id.xy);
    float2 positionScreen = float2(pixel) + 0.5f;
    float2 uv = positionScreen / float2(width, height);
    
    float depth = depthStencilTexture[pixel].r;
    
    float4 albedoMetallicSample = albedoMetallicTexture[pixel];
    float3 albedoColour = albedoMetallicSample.rgb;
    float metalllic = albedoMetallicSample.a;

    float4 normalRoughnessSample = normalRoughnessTexture[pixel];
    float3 normalV = normalize(normalRoughnessSample.rgb * 2.0f - 1.0f);
    bool roughness = normalRoughnessSample.a;
    
    float3 positionWorld = ReconstructWorldPosition(uv, depth);
    float3 viewV = normalize(cameraPosition - positionWorld);
    
    float3 ambient = ambientColour;
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < directionalLightCount; ++i)
    {
        float3 lightV = -directionalLights[i].direction;
        
        float diffuseFactor = dot(normalV, lightV);
        if (diffuseFactor <= 0.0f)
            continue;
        
        float3 radiance = directionalLights[i].colour;

        diffuse += radiance * diffuseFactor;

        float3 halfV = normalize(lightV + viewV);
        float specularFactor = saturate(dot(normalV, halfV));
        specular += radiance * pow(specularFactor, 70.0f);
    }
    
    float3 finalColour = (ambient + diffuse) * albedoColour + specular * float3(1, 1, 1);

    backBufferUAV[pixel] = float4(finalColour, 1.0f);
}