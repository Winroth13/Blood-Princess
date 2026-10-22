#include "gibs_component.hpp"
#include "scene/entity.hpp"

#include "core/rng.hpp"
#include "core/logging.hpp"
#include "scene/scene.hpp"

#include "components/core/pool_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/gameplay/splatter_component.hpp"
#include "components/core/model_component.hpp"
#include "components/core/lifetime_component.hpp"
#include "components/core/particle_system_component.hpp"

#include "assets/asset_manager.hpp"
#include "assets/types/model.hpp"

const std::vector<GibData> GibsComponent::GetGibModels(GibType type, uint32_t count)
{
    std::vector<GibData> gibData = {};
    gibData.reserve(count);

    switch (type)
    {
        case GibType::GOBLIN:
        {
            bool head = false, ribcage = false; // head and ribcages can only picked once

            for (uint32_t i = 0; i < count; ++i)
            {
                GibData data = {};

                float roll = RNG::Float();

                if (roll >= 0.85f && !head)
                {
                    data.model = this->gibModels[GOBLIN_HEAD];
                    data.scale = RNG::Float(0.3f, 0.4f);
                    head = true;
                }
                else if (roll >= 0.7f && !ribcage)
                {
                    data.model = this->gibModels[GOBLIN_RIBCAGE];
                    data.scale = RNG::Float(0.9f, 1.25f);
                    ribcage = true;
                }
                else
                {
                    int genericGib = RNG::Int(
                        static_cast<int>(GENERIC_GIB_START),
                        static_cast<int>(GENERIC_GIB_END)
                    );

                    if (genericGib == 0)
                        data.scale = RNG::Float(0.455f, 0.78f);
                    else
                        data.scale = RNG::Float(0.195f, 0.325f);

                    data.model = this->gibModels[genericGib];
                }

                gibData.push_back(data);
            }

        } break;
        case GibType::OGRE:
        {
            bool head = false, butt = false; // head and ribcages can only picked once

            for (uint32_t i = 0; i < count; ++i)
            {
                GibData data = {};

                float roll = RNG::Float();

                if (roll >= 0.85f && !head)
                {
                    data.model = this->gibModels[OGRE_HEAD];
                    data.scale = RNG::Float(0.5f, 0.7f);
                    head = true;
                }
                else if (roll >= 0.7f && !butt)
                {
                    data.model = this->gibModels[OGRE_BUTT];
                    data.scale = RNG::Float(1.2f, 1.55f);
                    butt = true;
                }
                else
                {

                    data.scale = RNG::Float(1.2f, 1.9f);

                    data.model = this->gibModels[OGRE_ARM];
                }

                gibData.push_back(data);
            }

        } break;
        case GibType::SKELETON:
        {
            bool head = false, bow = false; // head and ribcages can only picked once

            for (uint32_t i = 0; i < count; ++i)
            {
                GibData data = {};

                float roll = RNG::Float();

                if (roll >= 0.85f && !head)
                {
                    data.model = this->gibModels[SKELETON_HEAD];
                    data.scale = RNG::Float(2.0f, 2.0f);
                    head = true;
                }
                else if (roll >= 0.7f && !bow)
                {
                    data.model = this->gibModels[SKELETON_BOW];
                    data.scale = RNG::Float(1.2f, 1.55f);
                    bow = true;
                }
                else
                {

                    data.scale = RNG::Float(1.2f, 1.9f);
                    data.model = this->gibModels[SKELETON_ARM];
                }

                gibData.push_back(data);
            }

        } break;
        default:
        {
            LogWarn("Unsupported gib type: %d", static_cast<int>(type));
        } break;
    }

    return gibData;
}

void GibsComponent::OnStart(const AppContext &context)
{
    this->gibModels =
    {
        context.assetManager->Get<Model>("assets/models/gib1.glb"),
        context.assetManager->Get<Model>("assets/models/gib2.glb"),
        context.assetManager->Get<Model>("assets/models/gib3.glb"),
        context.assetManager->Get<Model>("assets/models/gib4.glb"),

        context.assetManager->Get<Model>("assets/models/ogregib1.glb"),
        context.assetManager->Get<Model>("assets/models/ogregib2.glb"),
        context.assetManager->Get<Model>("assets/models/ogregib3.glb"),

        context.assetManager->Get<Model>("assets/models/skelegib1.glb"),
        context.assetManager->Get<Model>("assets/models/skelegib2.glb"),
        context.assetManager->Get<Model>("assets/models/skelegib3.glb"),
    };

    this->bloodSpriteSheet = context.assetManager->Get<Texture2D>("assets/images/detailed_smoke_spritesheet.png");
}

void GibsComponent::SpawnGibs(XMFLOAT3 position, uint32_t count, GibType type)
{
    PoolComponent *pool = this->GetOwner()->GetComponent<PoolComponent>();

    auto data = this->GetGibModels(type, count);

    for (uint32_t i = 0; i < count; ++i)
    {
        Entity *gibEntity = pool->Grab();

        if (!gibEntity)
            return;

        ModelComponent *gibModel = gibEntity->GetComponent<ModelComponent>();
        gibModel->SetModel(data[i].model);

        TransformComponent *gibTransform = gibEntity->GetComponent<TransformComponent>();
        gibTransform->SetWorldPosition(position);
        gibTransform->SetLocalScale({data[i].scale, data[i].scale, data[i].scale});

        float lifeTime = RNG::Float(4.0f, 8.0f);

        LifeTimeComponent *lifeTimeGib = gibEntity->GetComponent<LifeTimeComponent>();
        lifeTimeGib->SetLifeTime(lifeTime);

        ProjectileComponent *projectileGib = gibEntity->GetComponent<ProjectileComponent>();
        projectileGib->SetAliveTime(lifeTime + 1.0f);
        const float gAngle = RNG::Float(0.0f, DirectX::XM_2PI);
        const float horizontalSpeed = RNG::Float(1.5f, 4.5f);
        const float upwardSpeed = RNG::Float(4.0f, 6.0f);

        DirectX::XMFLOAT3 velocity = {
            cosf(gAngle) * horizontalSpeed,
            upwardSpeed,
            sinf(gAngle) * horizontalSpeed
        };

        DirectX::XMFLOAT3 angularVelocity = {
            RNG::Float(-360.0f, 360.0f),
            RNG::Float(-360.0f, 360.0f),
            RNG::Float(-360.0f, 360.0f)
        };

        projectileGib->Launch(velocity, angularVelocity);
        gibEntity->GetComponent<SplatterComponent>()->Reset();
    }
}

void GibsComponent::SpawnBlood(Scene *scene, XMFLOAT3 position, float scale)
{
    Entity *bloodExplosionEntity = scene->AddEntity("");
    bloodExplosionEntity->AddComponent<TransformComponent>()->SetWorldPosition(position);

    ParticleSystemComponent *ps = bloodExplosionEntity->AddComponent<ParticleSystemComponent>();
    ps->SetSpawnRadius(RNG::Float(0.1f, 0.4f));
    ps->SetIsBurst(true);
    ps->SetIsSpread(true);
    ps->SetCount(static_cast<uint32_t>(20 * scale));
    ps->SetStartTint({ 0.35f, 0.03f, 0.03f });
    ps->SetEndTint({ 0.15f, 0.02f, 0.02f });
    ps->SetStartScale(0.8f * scale);
    ps->SetEndScale(0.8f * scale);
    ps->SetLifeTime(2.0f);

    ps->SetVelocity({ 4.0f * scale, 4.0f * scale, 4.0f * scale });
    ps->SetAcceleration({ 0.0f, -9.82f, 0.0f });

    ps->SetIsAnimate(true);
    ps->SetAtlasTexture(this->bloodSpriteSheet);
    ps->SetAtlasSize({ 8.0f, 8.0f });

    bloodExplosionEntity->AddComponent<LifeTimeComponent>()->SetLifeTime(2.0f);
}
