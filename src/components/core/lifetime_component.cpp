#include "lifetime_component.hpp"
#include "scene/entity.hpp"
#include "scene/scene.hpp"

void LifeTimeComponent::Update(const AppContext &context, float deltaTime)
{
	this->lifeTime -= deltaTime;

	if (this->lifeTime <= 0.0f)
	{
		auto scene = this->GetOwner()->GetScene();
		scene->DestroyEntity(this->GetOwner());
	}
}
