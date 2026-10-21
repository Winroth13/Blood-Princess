#include "level_component.hpp"

#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "components/gameplay/wave_controller.hpp"
#include "assets/types/floor.hpp"
#include "scene/scene_manager.hpp"
#include "core/application.hpp"

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
		this->OnLevelEnd();
		this->sceneManager->RequestSceneChange("win_screen");
	});

	this->playerController->GetOnComboEnd().Connect([this](uint32_t score)
	{
		this->GatherScore();
	});

	this->playerDiedConnection = EventBus::Subscribe<PlayerDiedEvent>(
		this, &LevelComponent::OnPlayerDied
	);

	waveController->StartFloor(context.assetManager->Get<Floor>("assets/floors/floor_one.yaml"));

	this->sceneManager = context.sceneManager;
	this->gameState = context.application->GetGameState();
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
	this->OnLevelEnd();
	this->sceneManager->RequestSceneChange("death_screen");
}

void LevelComponent::GatherScore()
{
	this->score += this->playerController->ClaimScore();
	this->onScoreChanged.Emit(this->score);
}

void LevelComponent::OnLevelEnd()
{
	this->GatherScore();
	gameState->addScore(this->score);
}