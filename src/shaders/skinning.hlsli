#ifndef SKINNING_HLSLI
#define SKINNING_HLSLI

#define MAX_SKIN_JOINT_COUNT 256

cbuffer Skinning : register(b8)
{
    float4x4 skinMatrices[MAX_SKIN_JOINT_COUNT];
}

float4x4 GetSkinMatrix(uint4 joints, float4 weights)
{
    return (
        skinMatrices[joints.x] * weights.x +
        skinMatrices[joints.y] * weights.y +
        skinMatrices[joints.z] * weights.z +
        skinMatrices[joints.w] * weights.w
    );
}

#endif