#pragma once
#include "scene/component.hpp"
#include "assets/types/floor.hpp"
#include "events/signal.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "core/game_state.hpp"

class LevelComponent : public Component
{
	RegisterComponent(LevelComponent);

	uint32_t score = 0;
	uint32_t numKills = 0;
	uint32_t numParries = 0;
	uint32_t topCombo = 0;

	PlayerControllerComponent *playerController = nullptr;

	ConnectionId playerDiedConnection = 0;

	SceneManager *sceneManager = nullptr;
	GameState *gameState = nullptr;
	Entity *endScreen = nullptr;

	Signal<uint32_t> onScoreChanged;

	const float END_TIMER_DURATION = 1.5f;
	float endTimer = -1.0f;

private:
	void OnStart(const AppContext &context) override;
	void OnDestroy(const AppContext &context) override;
	void Update(const AppContext &context, float deltaTime) override;

	void OnPlayerDied(const PlayerDiedEvent &event);
	void UpdateScore(uint32_t score, uint32_t combo);
	void OnLevelEnd();

public:
	LevelComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~LevelComponent() = default;

	void SetPlayerController(PlayerControllerComponent *playerController) { this->playerController = playerController; }

	void SetEndScreen(Entity *entity) { this->endScreen = entity; }

	uint32_t GetScore() const { return this->score; }
	uint32_t GetNumKills() const { return this->numKills; }
	uint32_t GetNumParries() const { return this->numParries; }
	uint32_t GetTopCombo() const { return this->topCombo; }

	Signal<uint32_t> &GetOnScoreChanged() { return this->onScoreChanged; }
};