cbuffer cbPerView : register(b0)
{
    float4x4 viewMatrix;
    float4x4 invViewMatrix;
    float4x4 projectionMatrix;
    float4x4 invProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 invViewProjectionMatrix;
    float3 cameraPosition;
    float pad0;
}

cbuffer cbPerObject : register(b1)
{
    float4x4 worldMatrix;
    float4x4 worldInvTransposeMatrix;
};

cbuffer cbPerMaterial : register(b2)
{
    float3 ambientCoefficient;
    float phongExponent;
    float3 diffuseCoefficient;
    float reflectiveness;
    float3 specularCoefficient;
    uint materialFlags;
    float maxTessFactor;
    float maxTessDistance;
    float minTessDistance;
    float dispStrength;
}