#include "tutorial_component.hpp"
#include "components/ui/ui_text_component.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "components/ai/enemy_controller_component.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/gameplay/wave_controller.hpp"
#include "components/collisions/hurt_box_component.hpp"
#include "input/action.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "core/application.hpp"
#include "core/game_state.hpp"

void TutorialComponent::Setup(Entity* panel, UITextComponent* textComp, EnemySpawnerComponent* spawner, PlayerControllerComponent* playerController, WaveController *waveController)
{
	this->popupPanel = panel;
	this->textComponent = textComp;
	this->enemySpawner = spawner;
	this->playerController = playerController;
	this->waveController = waveController;
}

void TutorialComponent::SetIsCompleted(bool isCompleted)
{
	this->isCompleted = isCompleted;
	this->manualCompletion = isCompleted;

	if (isCompleted)
		this->CompleteTutorial();
}


void TutorialComponent::OnStart(const AppContext& context)
{
	this->steps = {
		{ "Move with [W, A, S, D]", "MoveForward", 0.5f, 0.0f },
		{ "Dash with [Space] or [Shift]", "Dash", 0.0f, 0.0f },
		{ "Attack with [Left Click]", "LightAttack", 0.0f, 0.0f },
		{ "Heavy Attack with [Right Click]", "HeavyAttack", 0.0f, 0.0f },
		{ "Parry with [F], parry projectile to deflect", "Parry", 0.0f, 0.0f },
		{ "Survive the oncoming wave!", "", 0.0f, 3.0f }
	};

	this->currentStepIndex = 0;
	this->stepTimer = 0.0f;

	this->gameState = context.application->GetGameState();

	if (this->gameState->GetIsSkipTutorial())
		this->isCompleted = true;

	if (!this->isCompleted)
	{
		if (this->popupPanel)
		{
			this->popupPanel->SetLocallyActive(true);
		}

		this->ShowStep(0);

		this->attackHitConnection = this->enemySpawner->GetOnEnemyKilled().Connect([this](EnemyType type, uint32_t wave)
			{
				if (this->isCompleted)
					return;

				std::string actionName = this->steps[this->currentStepIndex].actionName;

				if (actionName == "LightAttack")
				{
					if (this->playerController->IsLightAttack())
					{
						this->currentStepDone = true;
						this->enemyToSpawn = EnemyType::GOBLIN;
					}
					else
					{
						this->enemyToSpawn = EnemyType::GOBLIN;
					}
				}
				else if (actionName == "HeavyAttack")
				{
					if (this->playerController->IsHeavyAttack())
					{
						this->currentStepDone = true;
						this->enemyToSpawn = EnemyType::SKELETON;
					}
					else
					{
						this->enemyToSpawn = EnemyType::GOBLIN;
					}
				}
			});

		this->attackHitConnection = EventBus::Subscribe<AttackHitEvent>(
			this, &TutorialComponent::OnAttackHit
		);

		this->hasConnetions = true;

		this->playerController->GetOwner()->GetComponent<HurtboxComponent>()->SetIsInvincible(true);
	}
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
					this->currentStepDone = true;
				}
			}
		}
		else if (step.actionName == "LightAttack" || step.actionName == "HeavyAttack" || step.actionName == "Parry")
		{}
		else
		{
			if (Action::IsActionPressed(step.actionName) || Action::IsActionDown(step.actionName))
			{
				this->currentStepDone = true;
				if (step.actionName == "Dash")
					this->enemyToSpawn = EnemyType::GOBLIN;
			}
		}
	}
	else if (step.duration > 0.0f)
	{
		if (this->stepTimer >= step.duration)
		{
			this->currentStepDone = true;
		}
	}

	if (this->currentStepDone)
	{
		this->currentStepIndex++;
		this->currentStepDone = false;
		if (this->currentStepIndex < this->steps.size())
		{
			this->ShowStep(this->currentStepIndex);
		}
		else
		{
			this->CompleteTutorial();
		}
	}

	if (this->enemyToSpawn != EnemyType::COUNT)
	{
		this->enemySpawner->SpawnEnemy(+enemyToSpawn, -1);
		this->enemyToSpawn = EnemyType::COUNT;
	}
}

void TutorialComponent::OnDestroy(const AppContext &context)
{
	this->RemoveConnections();
}

void TutorialComponent::CompleteTutorial()
{
	this->isCompleted = true;

	this->playerController->GetOwner()->GetComponent<HurtboxComponent>()->SetIsInvincible(false);

	if (this->popupPanel)
	{
		this->popupPanel->SetLocallyActive(false);
	}

	this->RemoveConnections();

	LogInfo("Tutorial completed! Starting enemy wave...");

	if (this->waveController && !this->manualCompletion)
	{
		this->waveController->NextWave();
	}
}

void TutorialComponent::RemoveConnections()
{
	if (!this->hasConnetions)
		return;

	if (this->attackHitConnection != 0)
	{
		EventBus::Unsubscribe<AttackHitEvent>(this->attackHitConnection);
		this->attackHitConnection = 0;
	}

	if (this->enemySpawner)
	{
		this->enemySpawner->GetOnEnemyKilled().DisconnectObject(this);
	}

	this->hasConnetions = false;
}

void TutorialComponent::OnAttackHit(const struct AttackHitEvent &event)
{
	if (this->isCompleted)
		return;

	std::string actionName = this->steps[this->currentStepIndex].actionName;

	EnemyControllerComponent *enemyController = event.victim->GetComponent<EnemyControllerComponent>();
	ProjectileComponent *projectile = event.attacker->GetComponent<ProjectileComponent>();

	if (actionName == "Parry" && enemyController)
	{
		if (projectile)
		{
			this->currentStepDone = true;
		}
		else if(event.victim->GetComponent<Health>()->IsDead())
		{
			this->enemyToSpawn = EnemyType::SKELETON;
		}
	}
}
