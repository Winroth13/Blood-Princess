#ifndef SPEED_LINES
#define SPEED_LINES

#include "../shared_resources.hlsli"

float InvLerp(float from, float to, float value)
{
    return (value - from) / (to - from);
}

float2 PolarCoordinates(float2 uv, float2 center, float zoom, float repeat)
{
    float2 dir = uv - center;
    float radius = length(dir) * 2.0f;
    float angle = atan2(dir.y, dir.x) * 1.0f / (PI * 2.0f);
    return float2(radius * zoom, angle * repeat) % 1.0f;
}

float2 RotateUv(float2 uv, float2 pivot, float rotation)
{
    float cosa = cos(rotation);
    float sina = sin(rotation);
    uv -= pivot;
    return float2(
        cosa * uv.x - sina * uv.y,
        cosa * uv.y + sina * uv.x 
    ) + pivot;
}

float4 SpeedLines(Texture2D<float4> noiseTexture, float2 uv)
{
    float2 polarUv = PolarCoordinates(RotateUv(uv, float2(0.5f, 0.5f), floor(frac(totalTime) * speedLineAnimationSpeed)), float2(0.5f, 0.5f), 0.01, speedLineCount);
    float3 lines = noiseTexture.SampleLevel(samplerLinearWrap, polarUv, 0).rgb;
	
    float maskValue = length(uv - float2(0.5f, 0.5f));
    float mask = InvLerp(speedLineMaskSize, speedLineMaskEdge, maskValue);
    float result = 1.0 - (mask * speedLineDensity);
	
    result = smoothstep(result, result + speedLineFaloff, lines.r);
    
    float4 colour;
    
    colour.rgb = float3(speedLineColour.rgb);
    colour.a = min(speedLineColour.a, result);
    
    return colour;
}

#endif // SPEED_LINES