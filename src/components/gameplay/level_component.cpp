#include "level_component.hpp"

#include "core/math.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "components/gameplay/wave_controller.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "assets/types/floor.hpp"
#include "scene/scene_manager.hpp"
#include "core/application.hpp"

void LevelComponent::OnStart(const AppContext &context)
{
	WaveController *waveController = this->GetOwner()->GetComponent<WaveController>();

	if (!waveController)
	{
		LogError("Must have a wave controller");
		return;
	}

	EnemySpawnerComponent *enemySpawner = this->GetOwner()->GetComponent<EnemySpawnerComponent>();

	if (!enemySpawner)
	{
		LogError("Must have an enemy spawner");
		return;
	}

	waveController->GetOnWaveCompleted().Connect([waveController]() {
		waveController->NextWave();
	});

	waveController->GetOnFloorCompleted().Connect([this]()
	{
		this->state = LevelState::SHOULD_COMPLETE;
	});

	enemySpawner->GetOnEnemyKilled().Connect([this](EnemyType type, uint32_t wave)
	{
		this->numKills++;
	});

	this->playerController->GetOnParry().Connect([this]()
	{
		this->numParries++;
	});

	this->playerController->GetOnComboEnd().Connect([this](uint32_t score, uint32_t combo)
	{
		UpdateScore(score, combo);
	});

	//this->playerController->GetOnLevelCompleted().Connect([this]()
	//{
	//	this->OnLevelEnd();
	//	this->sceneManager->RequestSceneChange("win_screen");
	//});

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

void LevelComponent::Update(const AppContext &context, float deltaTime)
{
	switch (this->state)
	{
		case LevelState::SHOULD_COMPLETE:
		{
			this->OnLevelEnd();
			this->state = LevelState::COMPLETED;
			this->endScreen->SetLocallyActive(true);
			this->endTimer = 0.0f;
		} break;
		case LevelState::COMPLETED:
		{
			if (this->endTimer >= 0.0f)
			{
				this->endTimer += deltaTime;
				float endRatio = this->endTimer / END_TIMER_DURATION;
				LogInfo("%f", endRatio);

				float alpha = 1.0f - Math::easeOutQuint(endRatio);

				context.application->SetTimeScale(alpha);
				context.audioManager->SetPitchModifier(alpha);
			}
		} break;
		default:
			break;
	}
}

void LevelComponent::OnPlayerDied(const PlayerDiedEvent &event)
{
	this->OnLevelEnd();
	this->sceneManager->RequestSceneChange("death_screen");
}

void LevelComponent::UpdateScore(uint32_t score, uint32_t combo)
{
	this->score += score;
	this->onScoreChanged.Emit(this->score);

	if (this->topCombo < combo)
		this->topCombo = combo;
}

void LevelComponent::OnLevelEnd()
{
	gameState->AddScore(this->score);
	gameState->AddKills(this->numKills);
	gameState->AddParries(this->numParries);
	gameState->SubmitTopCombo(this->topCombo);
}