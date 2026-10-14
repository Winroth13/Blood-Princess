#include "gibs_component.hpp"
#include "scene/entity.hpp"

#include "core/rng.hpp"
#include "core/logging.hpp"

#include "components/core/pool_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/gameplay/splatter_component.hpp"
#include "components/core/model_component.hpp"
#include "components/core/lifetime_component.hpp"

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
    };
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

        LifeTimeComponent *lifeTimeGib = gibEntity->GetComponent<LifeTimeComponent>();
        lifeTimeGib->SetLifeTime(RNG::Float(4.0f, 8.0f));

        ProjectileComponent *projectileGib = gibEntity->GetComponent<ProjectileComponent>();
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
