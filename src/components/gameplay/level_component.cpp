#include "level_component.hpp"

#include "core/math.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "components/gameplay/wave_controller.hpp"
#include "components/audio/audio_component.hpp"
#include "components/core/camera_component.hpp"
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

	waveController->GetOnFloorCompleted().Connect([this, context]() {
		this->OnLevelEnd();
		this->endScreen->SetLocallyActive(true);
		this->endTimer = 0.0f;
		this->GetOwner()->GetComponent<AudioComponent>()->Play();

		this->playerController->SetCanRotate(false);
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

void LevelComponent::Update(const AppContext &context, float deltaTime)
{
	if (this->endTimer >= 0.0f)
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
			context.audioManager->SetPitchModifier(alpha);
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