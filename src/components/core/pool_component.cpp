#include "pool_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

void PoolComponent::SetSize(int size)
{
	this->size = size;
	this->Resize();
}

Entity *PoolComponent::Grab()
{
	if (this->inactive.size() == 0)
	{
		LogWarn("Pool is empty, consider increasing it's size!");
		return nullptr;
	}

	Entity *entity = this->inactive.top();
	entity->SetLocallyActive(true);
	this->inactive.pop();

	return entity;
}

void PoolComponent::Put(Entity *entity)
{
	entity->SetLocallyActive(false);
	this->active.push(entity);
}

void PoolComponent::OnStart(const AppContext &context)
{
	this->Resize();
}

void PoolComponent::Resize()
{
	LogInfo("Resizing Pool to %d", this->size);
	
	if (!this->createEntityCallback)
		return;

	size_t totalSize = this->inactive.size() + this->active.size();

	if (this->size > totalSize)
	{
		for (size_t i = totalSize; i < this->size; ++i)
		{
			Entity *entity = this->createEntityCallback();

			if (!entity)
			{
				LogWarn("Created entity was null");
				continue;
			}

			entity->SetLocallyActive(false);
			entity->SetParent(this->GetOwner());
			this->inactive.push(entity);
		}
	}
}
