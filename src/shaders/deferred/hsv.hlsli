#ifndef HSV
#define HSV

#include "../shared_resources.hlsli"

float3 RgbToHsv(float3 rgb)
{
    float r = rgb.r;
    float g = rgb.g;
    float b = rgb.b;
	
    float max_val = max(r, max(g, b));
    float min_val = min(r, min(g, b));
    float delta = max_val - min_val;
	
    float h = 0.0;
    float s = 0.0;
    float v = max_val;
	
    if (delta > 0.0001)
    {
        s = delta / max_val;
		
        if (abs(r - max_val) < 0.0001)
            h = (g - b) / delta;
        else if (abs(g - max_val) < 0.0001)
            h = 2.0 + (b - r) / delta;
        else
            h = 4.0 + (r - g) / delta;
		
        h *= 60.0;
        if (h < 0.0)
            h += 360.0;
    }
	
    return float3(h, s, v);
}

float3 HsvToRgb(float3 hsv)
{
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;
	
    float c = v * s;
    float x = c * (1.0 - abs(((h / 60.0) % 2.0) - 1.0));
    float m = v - c;
	
    float3 rgb;
	
    if (h < 60.0)
        rgb = float3(c, x, 0.0);
    else if (h < 120.0)
        rgb = float3(x, c, 0.0);
    else if (h < 180.0)
        rgb = float3(0.0, c, x);
    else if (h < 240.0)
        rgb = float3(0.0, x, c);
    else if (h < 300.0)
        rgb = float3(x, 0.0, c);
    else
        rgb = float3(c, 0.0, x);
	
    rgb += m;
    return rgb;
}

#endif // HSV