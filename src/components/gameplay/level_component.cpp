#include "level_component.hpp"

#include "core/math.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "components/gameplay/wave_controller.hpp"
#include "components/audio/audio_component.hpp"
#include "components/core/camera_component.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "components/gameplay/level_endscreen_component.hpp"
#include "components/ui/ui_image_component.hpp"
#include "assets/types/floor.hpp"
#include "scene/scene_manager.hpp"
#include "core/math.hpp"
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

	auto endScreenComponent = this->endScreen->GetComponent<LevelEndscreenComponent>();
	endScreenComponent->GetOnAscend().Connect([this]() {
		if (this->state == LevelState::COMPLETED)
		{
			this->state = LevelState::ENDING;
			this->gameTimer = 0.0f;
		}
	});

	// Load the next floor and play it!
	std::string path = std::format("assets/floors/floor_{}.yaml", context.application->GetGameState()->GetNumFloorsComplete());
	auto floor = context.assetManager->Get<Floor>(path);
	waveController->StartFloor(floor);

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
	// *Sigh* has to be done this way since we are using slowmotion everywhere!
	if (this->state == LevelState::STARTING)
		this->gameTimer += deltaTime > 0.16f ? 0.16f : deltaTime;
	else if (this->state == LevelState::ENDING)
		this->gameTimer += deltaTime / context.application->GetTimeScale();
	else
		this->gameTimer += deltaTime;

	switch (this->state)
	{
		case LevelState::STARTING:
		{
			constexpr float FADE_IN_TIME = 4.0f;

			// This is bad but I can't be bothered to change what entity is sent in here so we just get the child.
			float alpha = 1 - Math::easeOutQuint(this->gameTimer / FADE_IN_TIME);
			UIImageComponent *fadeImage = this->fadeEntity->GetChildren()[0]->GetComponent<UIImageComponent>();
			fadeImage->SetColour({ 0.0f, 0.0f, 0.0f, alpha});

			if (this->gameTimer >= FADE_IN_TIME)
			{
				this->state = LevelState::RUNNING;
				fadeImage->SetColour({ 0.0f, 0.0f, 0.0f, 0.0f});
				this->gameTimer = 0;
			}

		} break;

		case LevelState::RUNNING:
		{
			
		} break;

		case LevelState::SHOULD_COMPLETE:
		{
			this->OnLevelEnd();
			this->state = LevelState::COMPLETED;
			this->endScreen->SetLocallyActive(true);
			this->endTimer = 0.0f;
			this->GetOwner()->GetComponent<AudioComponent>()->Play();
			this->playerController->SetCanRotate(false);

			auto endScreenComponent = this->endScreen->GetComponent<LevelEndscreenComponent>();
			endScreenComponent->SetTime(this->gameTimer);
			endScreenComponent->SetScore(this->score);
			endScreenComponent->SetKills(this->numKills);

			this->hudEntity->SetLocallyActive(false);
		} break;

		case LevelState::COMPLETED:
		{
			if (context.application->GetTimeScale() > 0.0001f)
				this->endTimer += deltaTime / context.application->GetTimeScale();

			float endRatio = this->endTimer / END_TIMER_DURATION;
			float alpha = 1.0f - Math::easeOutQuint(endRatio);

			context.application->SetTimeScale(alpha);

			Entity *player = this->playerController->GetOwner();
			Entity *viewmodel = player->GetChildren()[0];
			float fov = std::lerp(90.0f, 110.0f, 1 - alpha);
			viewmodel->GetChildren()[0]->GetComponent<CameraComponent>()->SetFov(DirectX::XMConvertToRadians(fov));
				
			if (alpha > 0.05f)
				context.audioManager->SetPitchModifier(std::clamp(alpha, 0.25f, 1.0f));

		} break;

		case LevelState::ENDING:
		{
			constexpr float FADE_OUT_TIME = 1.0f;

			// This is bad but I can't be bothered to change what entity is sent in here so we just get the child.
			float alpha = this->gameTimer / FADE_OUT_TIME;
			UIImageComponent *fadeImage = this->fadeEntity->GetChildren()[0]->GetComponent<UIImageComponent>();
			fadeImage->SetColour({ 0.0f, 0.0f, 0.0f, alpha});

			if (this->gameTimer >= FADE_OUT_TIME)
			{
				fadeImage->SetColour({ 0.0f, 0.0f, 0.0f, 1.0f});
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