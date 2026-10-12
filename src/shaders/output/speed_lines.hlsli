#ifndef SPEED_LINES
#define SPEED_LINES

#include "../shared_resources.hlsli"

//#define SPEEL_LINE_COLOUR float4(1.0f, 1.0f, 1.0f, 1.0f)
//#define SPEED_LINE_COUNT  2.0f // 0.0 - 2.0
//#define SPEED_LINE_DENSITY 0.4f // 0.0 - 1.0
//#define SPEED_LINE_FALLOFF 0.25f //0.0 - 1.0
//#define SPEED_MASK_SIZE 0.1f // 0.0 - 1.0
//#define SPEED_MASK_EDGE 0.5f // 0.0 - 1.0
//#define SPEED_ANIMATION_SPEED 10.0f // 1.0 - 20.0

float invLerp(float from, float to, float value)
{
    return (value - from) / (to - from);
}

float2 polarCoordinates(float2 uv, float2 center, float zoom, float repeat)
{
    float2 dir = uv - center;
    float radius = length(dir) * 2.0f;
    float angle = atan2(dir.y, dir.x) * 1.0f / (PI * 2.0f);
    return float2(radius * zoom, angle * repeat) % 1.0f;
}

float2 rotateUv(float2 uv, float2 pivot, float rotation)
{
    float cosa = cos(rotation);
    float sina = sin(rotation);
    uv -= pivot;
    return float2(
        cosa * uv.x - sina * uv.y,
        cosa * uv.y + sina * uv.x 
    ) + pivot;
}

float4 speedLines(Texture2D<float4> noiseTexture, float2 uv)
{
    float2 polarUv = polarCoordinates(rotateUv(uv, float2(0.5f, 0.5f), floor(frac(totalTime) * speedLineAnimationSpeed)), float2(0.5f, 0.5f), 0.01, speedLineCount);
    float3 lines = noiseTexture.SampleLevel(samplerLinearWrap, polarUv, 0).rgb;
	
    float maskValue = length(uv - float2(0.5f, 0.5f));
    float mask = invLerp(speedLineMaskSize, speedLineMaskEdge, maskValue);
    float result = 1.0 - (mask * speedLineDensity);
	
    result = smoothstep(result, result + speedLineFaloff, lines.r);
    
    float4 colour;
    
    colour.rgb = float3(speedLineColour.rgb);
    colour.a = min(speedLineColour.a, result);
    
    return colour;
}

#endif // SPEED_LINES