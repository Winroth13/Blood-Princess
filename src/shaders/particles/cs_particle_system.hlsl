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
    
    particle.velocity += acceleration * deltaTime;
    particle.position += particle.velocity * deltaTime;
    
    float lifeRatio = particle.lifeTime / lifeTime;
    
    bool shouldReset = (flags & PARTICLE_FLAG_RESET) == PARTICLE_FLAG_RESET;
    bool isBurst = (flags & PARTICLE_FLAG_BURST) == PARTICLE_FLAG_BURST;
    
    if (particle.lifeTime > lifeTime || shouldReset)
    {
        if (shouldReset)
        {
            particle.lifeTime = isBurst ? 0.0f : lifeTime * random(float2(DTid.x + ticks + 2, DTid.x + ticks + 2));
        }
        else
        {
            particle.lifeTime = particle.lifeTime % lifeTime;
        }
        
        lifeRatio = particle.lifeTime / lifeTime;
        
        float angle = random(float2(DTid.x + ticks, DTid.x + ticks)) * 3.14f * 2;
        float radiusRatio = random(float2(DTid.x + ticks + 1, DTid.x + ticks + 1));
        
        float x = cos(angle) * spawnRadius * radiusRatio;
        float z = sin(angle) * spawnRadius * radiusRatio;
        
        bool isSpread = (flags & PARTICLE_FLAG_SPREAD) == PARTICLE_FLAG_SPREAD;
        
        if (isSpread)
        {
            float randX = random(float2(DTid.x + ticks + 3, DTid.x + ticks + 3));
            float randY = random(float2(DTid.x + ticks + 5, DTid.x + ticks + 5));
            float randZ = random(float2(DTid.x + ticks + 4, DTid.x + ticks + 4));
            
            particle.velocity = float3(
                velocity.x * lerp(-1.0f, 1.0f, randX),
                velocity.y * lerp(-1.0f, 1.0f, randY),
                velocity.z * lerp(-1.0f, 1.0f, randZ)
            );
        }
        else
        {
            particle.velocity = velocity;

        }
        
        particle.position.x = x + particle.velocity.x * particle.lifeTime;
        particle.position.y = particle.velocity.y * particle.lifeTime;
        particle.position.z = z + particle.velocity.z * particle.lifeTime;
        
        if ((flags & PARTICLE_FLAG_GLOBAL) == PARTICLE_FLAG_GLOBAL)
        {
            particle.position += worldMatrix[3].xyz;
        }
    }
    
    particle.scale = lerp(startScale, endScale, lifeRatio);
    particle.tint = lerp(startTint, endTint, lifeRatio);
    
    particles[DTid.x] = particle;
}