#pragma once
#include "scene/component.hpp"
#include "assets/types/floor.hpp"
#include "events/signal.hpp"

struct ActiveWave
{
	bool completed = false;

	uint32_t totalScore = 0;
	uint32_t completionScore = 0;
	uint32_t currentScore = 0;

	std::array<int32_t, +EnemyType::COUNT> toSpawn = { 0 };
	std::array<float, +EnemyType::COUNT> spawnDuration = { 0.0f };
	std::array<float, +EnemyType::COUNT> spawnTimer = { 0.0f };
};

class WaveController : public Component
{
	RegisterComponent(WaveController);

	Signal<> onWaveCompleted;
	Signal<> onFloorCompleted;

private:
	std::shared_ptr<Floor> floor = nullptr;
	std::vector<ActiveWave> activeWaves = {};

	int32_t nextWave = 0;

	void CompleteWave(uint32_t waveIndex);
	float GetWaveProgress(ActiveWave &wave);

	bool IsFloorCompleted();

	void OnStart(const AppContext &context) override;
	void Update(const AppContext &context, float deltaTime) override;
	void Render(const RenderView &view, RenderQueue &queue) override;

public:
	WaveController(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); UsesRender(); };
	~WaveController() = default;

	void StartFloor(std::shared_ptr<Floor> floor);
	void NextWave();

	Signal<> &GetOnWaveCompleted() { return this->onWaveCompleted; }
	Signal<> &GetOnFloorCompleted() { return this->onFloorCompleted; }
};