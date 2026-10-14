#include "lifetime_component.hpp"
#include "components/core/pool_component.hpp"

#include "scene/entity.hpp"
#include "scene/scene.hpp"

void LifeTimeComponent::Update(const AppContext &context, float deltaTime)
{
	this->lifeTime -= deltaTime;

	if (this->lifeTime <= 0.0f)
	{
		auto poolComp = this->GetOwner()->GetComponent<PoolComponent>();

		if (poolComp)
		{
			poolComp->Put(this->GetOwner());
		}
		else
		{
			auto scene = this->GetOwner()->GetScene();
			scene->DestroyEntity(this->GetOwner());
		}
	}
}
