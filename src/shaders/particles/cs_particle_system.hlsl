#include "../shared_resources.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);

/*
*   Pseudo-Random Number for GPU
*   Found here: https://stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader
*/
float random(float2 p)
{
    float2 K1 = float2(
        23.14069263277926, // e^pi (Gelfond's constant)
         2.665144142690225 // 2^sqrt(2) (Gelfond-Schneider constant)
    );
    return frac(cos(dot(p, K1)) * 12345.6789);
}

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Particle particle = particles[DTid.x];
    
    particle.lifeTime += deltaTime;
    
    float3 realVelocity = velocity + acceleration * particle.lifeTime;
    particle.position += realVelocity * deltaTime;
   
    bool shouldReset = (flags & PARTICLE_FLAG_RESET) == PARTICLE_FLAG_RESET;
    
    if (particle.lifeTime > lifeTime || shouldReset)
    {
        particle.lifeTime = 0.0f;
        
        float angle = random(float2(DTid.x + ticks, DTid.x + ticks)) * 3.14f * 2;
        float radiusRatio = random(float2(DTid.x + ticks + 1, DTid.x + ticks + 1));
        
        float x = cos(angle) * spawnRadius * radiusRatio;
        float z = sin(angle) * spawnRadius * radiusRatio;
        
        particle.position.x = x + realVelocity.x * particle.lifeTime;
        particle.position.y = realVelocity.y * particle.lifeTime;
        particle.position.z = z + realVelocity.z * particle.lifeTime;
    }
    
    float lifeRatio = particle.lifeTime / lifeTime;
    particle.scale = lerp(startScale, endScale, lifeRatio);
    particle.tint = lerp(startTint, endTint, lifeRatio);
    
    particles[DTid.x] = particle;
}