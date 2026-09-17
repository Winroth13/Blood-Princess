SamplerState samplerLinearWrap : register(s0);

cbuffer PerView : register(b0)
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

cbuffer PerObject : register(b1)
{
    float4x4 worldMatrix;
    float4x4 worldInvTransposeMatrix;
};

cbuffer PerMaterial : register(b2)
{
    float3 materialTint;
    float materialSpecular;
    float materialSpecularExponent;
    float3 pad1;
}