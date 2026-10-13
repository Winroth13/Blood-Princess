#pragma once

#include "scene/component.hpp"
#include <string>
#include <vector>

class Entity;
class UITextComponent;
class EnemySpawnerComponent;

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
	EnemySpawnerComponent* enemySpawner = nullptr;

	std::vector<TutorialStep> steps;
	size_t currentStepIndex = 0;
	float stepTimer = 0.0f;
	bool isCompleted = false;

public:
	TutorialComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }
	~TutorialComponent() = default;

	void Setup(Entity* panel, UITextComponent* textComp, EnemySpawnerComponent* spawner);

private:
	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;

	void ShowStep(size_t index);
	void CompleteTutorial();
};
