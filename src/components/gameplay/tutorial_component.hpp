#pragma once

#include "scene/component.hpp"
#include "events/event_bus.hpp"
#include "assets/types/enemy.hpp"

#include <string>
#include <vector>

class Entity;
class UITextComponent;
class EnemySpawnerComponent;
class PlayerControllerComponent;
class WaveController;
class GameState;

struct TutorialStep
{
	std::string instruction;
	std::string actionName;       // Action name to check (e.g. "MoveForward", "LightAttack")
	float timeRequired = 0.0f;    // If action-based, can also require holding or instantaneous press
	float duration = 0.0f;        // Fallback or display duration if no actionName specified
};

class TutorialComponent : public Component
{
	RegisterComponent(TutorialComponent);

	Entity* popupPanel = nullptr;
	UITextComponent* textComponent = nullptr;
	EnemySpawnerComponent *enemySpawner = nullptr;
	PlayerControllerComponent* playerController = nullptr;
	WaveController* waveController = nullptr;
	GameState* gameState = nullptr;

	ConnectionId attackHitConnection = 0;

	std::vector<TutorialStep> steps;
	size_t currentStepIndex = 0;
	float stepTimer = 0.0f;
	bool isCompleted = false;
	bool currentStepDone = false;
	bool hasConnetions = false;
	bool manualCompletion = false;
	EnemyType enemyToSpawn = EnemyType::COUNT;

public:
	TutorialComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }
	~TutorialComponent() = default;

	void Setup(Entity* panel, UITextComponent* textComp, EnemySpawnerComponent* spawner, PlayerControllerComponent* playerController, WaveController* waveController);

	void SetIsCompleted(bool isCompleted);

private:
	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void OnDestroy(const AppContext &context) override;

	void RemoveConnections();

	void OnAttackHit(const struct AttackHitEvent &event);

	void ShowStep(size_t index);
	void CompleteTutorial();
};
