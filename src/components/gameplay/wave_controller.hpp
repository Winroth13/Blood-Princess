#pragma once
#include "scene/component.hpp"

class Floor;

struct ActiveWave
{
	std::array<>
};

class WaveController : public Component
{
	RegisterComponent(WaveController);

private:
	std::shared_ptr<Floor> floor = nullptr;

	int32_t currentWave = -1;
	uint32_t enemiesLeft = 0;

	void NextWave();

	void OnStart(const AppContext &context) override;
	void Update(const AppContext &context, float deltaTime) override;

public:
	WaveController(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~WaveController() = default;

	void SetFloor(std::shared_ptr<Floor> floor) { this->floor = floor; }
};