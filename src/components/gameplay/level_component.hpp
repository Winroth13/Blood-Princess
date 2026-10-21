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

	PlayerControllerComponent *playerController = nullptr;

	ConnectionId playerDiedConnection = 0;

	SceneManager *sceneManager = nullptr;
	GameState *gameState = nullptr;

	Signal<uint32_t> onScoreChanged;

private:
	void OnStart(const AppContext &context) override;
	void OnDestroy(const AppContext &context) override;

	void OnPlayerDied(const PlayerDiedEvent &event);
	void GatherScore();
	void OnLevelEnd();

public:
	LevelComponent(Entity *owner, bool isActive) : Component(owner, isActive) { };
	~LevelComponent() = default;

	void SetPlayerController(PlayerControllerComponent *playerController) { this->playerController = playerController; }

	uint32_t GetScore() const { return this->score; }

	Signal<uint32_t> &GetOnScoreChanged() { return this->onScoreChanged; }
};