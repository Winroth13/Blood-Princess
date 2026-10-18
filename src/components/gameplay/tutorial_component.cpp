#include "tutorial_component.hpp"
#include "components/ui/ui_text_component.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "input/action.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"

void TutorialComponent::Setup(Entity* panel, UITextComponent* textComp, EnemySpawnerComponent* spawner)
{
	this->popupPanel = panel;
	this->textComponent = textComp;
	this->enemySpawner = spawner;
}

void TutorialComponent::OnStart(const AppContext& context)
{
	this->steps = {
		{ "Move with [W, A, S, D]", "MoveForward", 0.5f, 0.0f },
		{ "Dash with [Space] or [Shift]", "Dash", 0.0f, 0.0f },
		{ "Attack with [Left Click]", "LightAttack", 0.0f, 0.0f },
		{ "Heavy Attack with [Right Click]", "HeavyAttack", 0.0f, 0.0f },
		{ "Parry with [F]", "Parry", 0.0f, 0.0f },
		{ "Survive the oncoming wave!", "", 0.0f, 3.0f }
	};

	this->currentStepIndex = 0;
	this->stepTimer = 0.0f;
	this->isCompleted = false;

	if (this->popupPanel)
	{
		this->popupPanel->SetLocallyActive(true);
	}

	this->ShowStep(0);
}

void TutorialComponent::ShowStep(size_t index)
{
	if (index >= this->steps.size())
	{
		this->CompleteTutorial();
		return;
	}

	this->stepTimer = 0.0f;
	if (this->textComponent)
	{
		this->textComponent->SetText(this->steps[index].instruction);
	}
}

void TutorialComponent::Update(const AppContext& context, float deltaTime)
{
	if (this->isCompleted || this->currentStepIndex >= this->steps.size())
	{
		return;
	}

	const TutorialStep& step = this->steps[this->currentStepIndex];
	this->stepTimer += deltaTime;

	bool stepDone = false;

	if (!step.actionName.empty())
	{
		if (step.actionName == "MoveForward")
		{
			// Check any movement key
			if (Action::IsActionDown("MoveForward") || Action::IsActionDown("MoveBackward") ||
				Action::IsActionDown("MoveLeft") || Action::IsActionDown("MoveRight"))
			{
				if (this->stepTimer >= step.timeRequired)
				{
					stepDone = true;
				}
			}
		}
		else
		{
			if (Action::IsActionPressed(step.actionName) || Action::IsActionDown(step.actionName))
			{
				stepDone = true;
			}
		}
	}
	else if (step.duration > 0.0f)
	{
		if (this->stepTimer >= step.duration)
		{
			stepDone = true;
		}
	}

	if (stepDone)
	{
		this->currentStepIndex++;
		if (this->currentStepIndex < this->steps.size())
		{
			this->ShowStep(this->currentStepIndex);
		}
		else
		{
			this->CompleteTutorial();
		}
	}
}

void TutorialComponent::CompleteTutorial()
{
	this->isCompleted = true;

	if (this->popupPanel)
	{
		this->popupPanel->SetLocallyActive(false);
	}

	if (this->enemySpawner)
	{
		LogInfo("Tutorial completed! Starting enemy wave...");
		//this->enemySpawner->BeginWave();
	}
}
