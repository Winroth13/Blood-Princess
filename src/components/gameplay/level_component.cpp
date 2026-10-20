#include "level_component.hpp"

#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "components/gameplay/wave_controller.hpp"
#include "assets/types/floor.hpp"

void LevelComponent::OnStart(const AppContext &context)
{
	auto *waveController = this->GetOwner()->GetComponent<WaveController>();

	if (!waveController)
	{
		LogError("Must have a wave controller");
		return;
	}

	waveController->GetOnWaveCompleted().Connect([waveController]() {
		waveController->NextWave();
	});

	waveController->GetOnFloorCompleted().Connect([this]() {
		this->GatherScore();
		LogInfo("Floor Completed!");
	});

	this->playerController->GetOnComboEnd().Connect([this](uint32_t score)
	{
		this->GatherScore();
	});

	this->playerDiedConnection = EventBus::Subscribe<PlayerDiedEvent>(
		this, &LevelComponent::OnPlayerDied
	);

	waveController->StartFloor(context.assetManager->Get<Floor>("assets/floors/floor_one.yaml"));
}

void LevelComponent::OnDestroy(const AppContext &context)
{
	if (this->playerDiedConnection != 0)
	{
		EventBus::Unsubscribe<PlayerDiedEvent>(this->playerDiedConnection);
		this->playerDiedConnection = 0;
	}
}

void LevelComponent::OnPlayerDied(const PlayerDiedEvent &event)
{
	this->GatherScore();
}

void LevelComponent::GatherScore()
{
	this->score += this->playerController->ClaimScore();
	this->onScoreChanged.Emit(this->score);
}