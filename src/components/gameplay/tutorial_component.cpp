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
#include <format>

static std::string GetSlotTutorialName(const ActionSlot& slot)
{
	if (slot.IsKey())
	{
		Key k = slot.AsKey();
		if (k == Key::UNKNOWN) return "";
		if (k == Key::LEFT_SHIFT || k == Key::RIGHT_SHIFT) return "Shift";
		if (k == Key::ENTER) return "Enter";
		const char* name = Input::GetKeyName(k);
		return name ? name : "";
	}
	else if (slot.IsMouse())
	{
		switch (slot.AsMouse())
		{
		case MouseButton::LEFT: return "Left Click";
		case MouseButton::RIGHT: return "Right Click";
		case MouseButton::MIDDLE: return "Middle Click";
		case MouseButton::X1: return "Mouse 4";
		case MouseButton::X2: return "Mouse 5";
		default: return "";
		}
	}
	return "";
}

static std::string GetActionSlot1Symbol(const std::string& actionName, const std::string& fallback)
{
	const ActionBinding* binding = Action::GetBinding(actionName);
	if (!binding) return fallback;

	std::string s1 = GetSlotTutorialName(binding->input1);
	if (!s1.empty()) return s1;

	std::string s2 = GetSlotTutorialName(binding->input2);
	if (!s2.empty()) return s2;

	if (binding->controllerInput != GamepadButton::UNKNOWN)
	{
		return Action::GetGamepadButtonDisplayName(binding->controllerInput);
	}

	return fallback;
}

static std::string GetActionConsoleSymbol(const std::string& actionName, const std::string& fallback)
{
	const ActionBinding* binding = Action::GetBinding(actionName);
	if (binding && binding->controllerInput != GamepadButton::UNKNOWN)
	{
		return Action::GetGamepadButtonDisplayName(binding->controllerInput);
	}
	return fallback;
}

void TutorialComponent::Setup(Entity* panel, UITextComponent* textComp, EnemySpawnerComponent* spawner, PlayerControllerComponent* playerController, WaveController *waveController)
{
	this->popupPanel = panel;
	this->textComponent = textComp;
	this->enemySpawner = spawner;
	this->playerController = playerController;
	this->waveController = waveController;
}

void TutorialComponent::QueueCompletion()
{
	this->isToComplete = true;
	this->manualCompletion = true;
}

void TutorialComponent::UpdateStepInstructions()
{
	if (this->steps.empty())
		return;

	if (this->isUsingGamepad)
	{
		const ActionBinding* fwdBinding = Action::GetBinding("MoveForward");
		if (fwdBinding && fwdBinding->controllerInput != GamepadButton::UNKNOWN)
		{
			std::string fwd = Action::GetGamepadButtonDisplayName(fwdBinding->controllerInput);
			std::string left = GetActionConsoleSymbol("MoveLeft", "D-Pad Left");
			std::string back = GetActionConsoleSymbol("MoveBackward", "D-Pad Down");
			std::string right = GetActionConsoleSymbol("MoveRight", "D-Pad Right");
			this->steps[0].instruction = std::format("Move with [{}, {}, {}, {}]", fwd, left, back, right);
		}
		else
		{
			this->steps[0].instruction = "Move with [Left Stick]";
		}

		std::string dash = GetActionConsoleSymbol("Dash", "RB / L1");
		this->steps[1].instruction = std::format("Dash with [{}]", dash);

		std::string atk = GetActionConsoleSymbol("LightAttack", "RT / R2");
		this->steps[2].instruction = std::format("Attack with [{}]", atk);

		std::string heavy = GetActionConsoleSymbol("HeavyAttack", "RB / R1");
		this->steps[3].instruction = std::format("Heavy Attack with [{}]", heavy);

		std::string parry = GetActionConsoleSymbol("Parry", "RT / L2");
		this->steps[4].instruction = std::format("Parry with [{}], parry projectile to deflect", parry);
	}
	else
	{
		std::string fwd = GetActionSlot1Symbol("MoveForward", "W");
		std::string left = GetActionSlot1Symbol("MoveLeft", "A");
		std::string back = GetActionSlot1Symbol("MoveBackward", "S");
		std::string right = GetActionSlot1Symbol("MoveRight", "D");
		this->steps[0].instruction = std::format("Move with [{}, {}, {}, {}]", fwd, left, back, right);

		const ActionBinding* dashBinding = Action::GetBinding("Dash");
		std::string dash1 = dashBinding ? GetSlotTutorialName(dashBinding->input1) : "";
		std::string dash2 = dashBinding ? GetSlotTutorialName(dashBinding->input2) : "";
		if (!dash1.empty() && !dash2.empty() && dash1 != dash2)
		{
			this->steps[1].instruction = std::format("Dash with [{}] or [{}]", dash1, dash2);
		}
		else if (!dash1.empty())
		{
			this->steps[1].instruction = std::format("Dash with [{}]", dash1);
		}
		else if (!dash2.empty())
		{
			this->steps[1].instruction = std::format("Dash with [{}]", dash2);
		}
		else
		{
			this->steps[1].instruction = "Dash with [Space] or [Shift]";
		}

		std::string atk = GetActionSlot1Symbol("LightAttack", "Left Click");
		this->steps[2].instruction = std::format("Attack with [{}]", atk);

		std::string heavy = GetActionSlot1Symbol("HeavyAttack", "Right Click");
		this->steps[3].instruction = std::format("Heavy Attack with [{}]", heavy);

		std::string parry = GetActionSlot1Symbol("Parry", "F");
		this->steps[4].instruction = std::format("Parry with [{}], parry projectile to deflect", parry);
	}

	if (this->steps.size() > 5)
	{
		this->steps[5].instruction = "Survive the oncoming wave!";
	}
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
	this->UpdateStepInstructions();

	this->currentStepIndex = 0;
	this->stepTimer = 0.0f;

	this->app = context.application;

	if (app->IsSkipTutorial() && app->HasCompletedTutorialOnce())
		this->isCompleted = true;

	if (this->isCompleted)
	{
		if (this->popupPanel)
		{
			this->popupPanel->SetLocallyActive(false);
		}
	}
	else
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
						this->enemyToSpawn = EnemyType::OGRE;
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

	this->UpdateStepInstructions();

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

	if (this->isToComplete)
	{
		this->CompleteTutorial();
		return;
	}

	bool prevUsingGamepad = this->isUsingGamepad;

	if (Input::GetAnyGamepadButtonPressed() != GamepadButton::UNKNOWN)
	{
		this->isUsingGamepad = true;
	}
	else
	{
		DirectX::XMFLOAT2 stick = Input::GetGamepadLeftStick();
		if (std::abs(stick.x) > 0.3f || std::abs(stick.y) > 0.3f)
		{
			this->isUsingGamepad = true;
		}
		else if (Input::GetGamepadAxis(GamepadAxis::LEFT_TRIGGER) > 0.3f || Input::GetGamepadAxis(GamepadAxis::RIGHT_TRIGGER) > 0.3f)
		{
			this->isUsingGamepad = true;
		}
	}

	if (Input::GetAnyKeyPressed() != Key::UNKNOWN || Input::GetAnyMouseButtonPressed() != MouseButton::UNKNOWN)
	{
		this->isUsingGamepad = false;
	}

	if (this->isUsingGamepad != prevUsingGamepad)
	{
		this->UpdateStepInstructions();
		if (this->textComponent && this->currentStepIndex < this->steps.size())
		{
			this->textComponent->SetText(this->steps[this->currentStepIndex].instruction);
		}
	}

	const TutorialStep& step = this->steps[this->currentStepIndex];
	this->stepTimer += deltaTime;

	if (!step.actionName.empty())
	{
		if (step.actionName == "MoveForward")
		{
			// Check any movement key
			DirectX::XMFLOAT2 stick = Input::GetGamepadLeftStick();
			bool stickMoved = (std::abs(stick.x) > 0.2f || std::abs(stick.y) > 0.2f);
			if (Action::IsActionDown("MoveForward") || Action::IsActionDown("MoveBackward") ||
				Action::IsActionDown("MoveLeft") || Action::IsActionDown("MoveRight") || stickMoved)
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

	this->app->SetCompletedTutorial(true);

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
