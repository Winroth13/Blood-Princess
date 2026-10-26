#include "player_controller_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/core/transform_component.hpp"
#include "components/core/camera_component.hpp"
#include "input/input.hpp"
#include "input/action.hpp"
#include <algorithm>
#include "collision/collision_manager.hpp"
#include "components/audio/audio_component.hpp"
#include "audio/audio_manager.hpp"
#include "core/rng.hpp"
#include "scene/scene.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "components/audio/character_audio_component.hpp"
#include "components/skeletal/animator_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "assets/types/sound.hpp"
#include "assets/asset_manager.hpp"
#include "components/audio/sound_component.hpp"
#include "events/event_bus.hpp"
#include "core/debug.hpp"
#include "components/gameplay/health_component.hpp"
#include "components/collisions/hurt_box_component.hpp"
#include "components/gameplay/shake_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/core/lifetime_component.hpp"
#include "components/core/particle_system_component.hpp"
#include "components/lights/point_light_component.hpp"
#include "components/ai/enemy_controller_component.hpp"
#include "core/application.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "components/gameplay/enemy_database_component.hpp"
#include "components/gameplay/wave_controller.hpp"
#include "collision/collision_component.hpp"
#include "core/math.hpp"
#include "assets/types/audio_clip.hpp"
#include "components/gameplay/viewbob_component.hpp"

#include <cmath>

void PlayerControllerComponent::OnStart(const AppContext& context)
{
	this->application = context.application;
	if (this->application)
	{
		float sens = this->application->GetMouseSensitivity();
		this->mouseSensitivity = { sens, sens };
	}
	this->audioManager = context.audioManager;
	this->collisionRef = context.collisionManager;
	this->GetOwner()->GetScene()->AddEntityToGroup(this->groupName, this->GetOwner());
	this->health = this->GetOwner()->GetComponent<Health>();
	this->hurtbox = this->GetOwner()->GetComponent<HurtboxComponent>();

	if (this->hurtbox)
	{
		this->hurtbox->GetOnHitBlocked().Connect(
			this, &PlayerControllerComponent::OnHitBlocked);
	}

	if (this->health)
	{
		this->health->SetMaxHealth(this->maxHealth);
		this->health->SetCurrentHealth(this->maxHealth);
	}

	this->attackHitConnection = EventBus::Subscribe<AttackHitEvent>(
		this, &PlayerControllerComponent::OnAttackHit
	);

	this->attackShoutSound = context.assetManager->Get<Sound>("assets/sounds/attack_shout.sound");
	this->hurtSound = context.assetManager->Get<Sound>("assets/sounds/player_hurt.sound");
	this->parryShoutSound = context.assetManager->Get<Sound>("assets/sounds/player_parry_shout.sound");
    this->sfxDash = context.assetManager->Get<AudioClip>("assets/audio/sfx/Other/737748__squirrel_404__large-sword-swing.ogg");

	this->sparkTexture = context.assetManager->Get<Texture2D>("assets/images/sparks_new.png");
	this->parryShockwaveTexture = context.assetManager->Get<Texture2D>("assets/images/parry_shockwave.png");

	this->damageTakenConnection = EventBus::Subscribe<DamageTakenEvent>(
		this, &PlayerControllerComponent::OnDamageTaken
	);

	health->GetOnChangedHealth().Connect([this](float currentHealth, float maxHealth) {
		this->damageFlashLingeringTimer = this->damageFlashDuration;

		if (this->damageSoundReminingTime == -1.0f)
		{
			SoundComponent *soundComp = this->GetOwner()->GetComponent<SoundComponent>();
			soundComp->SetSound(this->hurtSound);
			soundComp->Play();
			this->damageSoundReminingTime = this->damageSoundDuration;
		}
		});

	Input::SetMouseMode(MouseMode::CAPTURED);

    this->dashCooldownTimer = this->dashCooldownDuration;

	if (!this->ConnectComboSignals())
		return;
}

void PlayerControllerComponent::Update(const AppContext& context, float deltaTime)
{
	if (this->isDead)
		return;

    if (Input::IsKeyPressed(Key::ESCAPE))
        this->SetIsPaused(!this->isPaused);

    if (this->isDoingHeavyAttack) {
        this->heavyAttackTimer += deltaTime;
        if (this->heavyAttackTimer >= this->HEAVY_ATTACK_DURATION || (Action::IsActionPressed("Parry") && this->heavyAttackTimer < 0.4f))
            this->isDoingHeavyAttack = false;
    }

	if (!this->canUseDash)
	{
		this->dashCooldownTimer -= deltaTime;
        this->onDashCooldownChanged.Emit(this->dashCooldownTimer);

		if (this->dashCooldownTimer <= 0)
		{
			this->canUseDash = true;
			this->dashCooldownTimer = this->dashCooldownDuration;
		}
	}

	if (this->isDashing)
	{
		PerformDash(deltaTime, this->dashDistance, this->dashDuration);
	}
	else
	{
		if (this->lingeringDashTimer >= 0.0f)
		{
			DirectX::XMFLOAT4 dashColour = Debug::GetSetting("speedLines.colour", DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.0f });

			float ligneringDashFactor = this->lingeringDashTimer / this->lingeringDashDuration;
			
			dashColour.w = std::expf(4 * (ligneringDashFactor - 1)) * this->maxDashAlpha;
			//dashColour.w = 1 - std::expf(-4 * ligneringDashFactor);
			//dashColour.w = std::logf(ligneringDashFactor);
			//dashColour.w = std::pow(ligneringDashFactor, 0.5f);
			Debug::SetSetting("speedLines.colour", dashColour);
			this->lingeringDashTimer -= deltaTime;

			float dashDensity = std::lerp(this->minDashDensity, this->maxDashDensity, ligneringDashFactor);
			Debug::SetSetting("speedLines.density", dashDensity);

			float fov = std::lerp(DirectX::XM_PIDIV2, this->dashFov, ligneringDashFactor);
			this->camera->SetFov(fov);
		}
		else
		{
			// only reset the dash once
			if (this->lingeringDashTimer != -1.0f)
			{
				this->lingeringDashTimer = -1;
				DirectX::XMFLOAT4 dashColour = Debug::GetSetting("speedLines.colour", DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.0f });
				dashColour.w = 0.0f;
				Debug::SetSetting("speedLines.colour", dashColour);
				Debug::SetSetting("speedLines.density", this->minDashDensity);
				this->camera->SetFov(DirectX::XM_PIDIV2);
			}
		}

		HandleMovement(deltaTime);
		HandleCombat(deltaTime); 
	}

	float hitFlash = 0.0f;
	if (this->damageFlashLingeringTimer >= 0.0f)
	{
		hitFlash = this->damageFlashLingeringTimer / this->damageFlashDuration;
		this->damageFlashLingeringTimer -= deltaTime;
	}
	else
	{
		this->damageFlashLingeringTimer = -1.0f;
	}

	float lowHealthFlash = 0.0f;
	if (this->health && this->health->GetMaxHealth() > 0.0f)
	{
		float healthRatio = this->health->GetCurrentHealth() / this->health->GetMaxHealth();
		float lowHealth = std::clamp(1.0f - healthRatio / this->lowHealthThreshold, 0.0f, 1.0f);

		this->lowHealthPulseTimer += deltaTime;
		float pulse = 0.6f + 0.4f * std::sin(this->lowHealthPulseTimer * this->lowHealthPulseSpeed);
		lowHealthFlash = lowHealth * pulse * this->lowHealthFlashIntensity;
	}

	Debug::SetSetting("damageFlash.intensity", (std::max)(hitFlash, lowHealthFlash));

	if (this->damageSoundReminingTime >= 0)
		this->damageSoundReminingTime -= deltaTime;
	else
		this->damageSoundReminingTime = -1.0f;

	HandleCameraLook(deltaTime);

	HandleAttacks(deltaTime);

	/* Handle Slowmotion */
	if (this->slowMotionTimer >= 0.0f)
	{
		// this timer shouldn't be affected by timescale
		this->slowMotionTimer -= deltaTime / this->application->GetTimeScale();

		float alpha = 1.0f - Math::easeInOutQuint(this->slowMotionTimer / SLOW_MOTION_TIME);
		this->application->SetTimeScale(std::lerp(this->slowMotionMin, 1.0f, alpha));
		this->camera->SetFov(XMConvertToRadians(std::lerp(this->PARRY_FOV, 90.0f, alpha)));
		if (this->audioManager)
			this->audioManager->SetPitchModifier(std::lerp(0.4f, 1.0f, alpha));
	}
	/*else if (this->audioManager && this->audioManager->GetPitchModifier() != 1.0f)
	{
		this->audioManager->SetPitchModifier(1.0f);
	}*/

	if (this->comboRemainingTime != -1.0f)
	{
		if (this->comboRemainingTime < deltaTime)
		{
			this->comboRemainingTime = -1.0f;
			this->onComboRemainingTimeChanged.Emit(0.0f);
			this->EndCombo();
		}
		else
		{
			this->comboRemainingTime -= deltaTime;
			this->onComboRemainingTimeChanged.Emit(this->comboRemainingTime);
		}
	}

	/* Handle Parry Reset */
	if (this->parryCountResetTimer > 0.0f)
	{
		this->parryCountResetTimer -= deltaTime;

		if (this->parryCountResetTimer <= 0.0f)
		{
			this->parryCount = 0;
		}
	}

	if (this->viewmodelAnimator->IsFinished())
	{
		this->viewmodelAnimator->CrossFade("idle", 0.2f);
	}
}

void PlayerControllerComponent::OnDestroy(const AppContext& context)
{
	if (this->attackHitConnection != 0)
	{
		EventBus::Unsubscribe<AttackHitEvent>(this->attackHitConnection);
		this->attackHitConnection = 0;
	}
	if (this->damageTakenConnection != 0)
	{
		EventBus::Unsubscribe<DamageTakenEvent>(this->damageTakenConnection);
		this->damageTakenConnection = 0;
	}

	this->GetOwner()->GetScene()->RemoveEntityFromGroup(this->groupName, this->GetOwner());

	if (this->hurtbox)
	{
		this->hurtbox->GetOnHitBlocked().DisconnectObject(this);
	}

	Debug::SetSetting("damageFlash.intensity", 0.0f);
}

void PlayerControllerComponent::GetDashDirection(XMVECTOR &direction)
{
    if (this->isPaused)
        return;

	auto transform = GetOwner()->GetComponent<TransformComponent>();
	float yawRad = XMConvertToRadians(this->yaw);

	DirectX::XMVECTOR forward = XMVectorSet(sinf(yawRad), 0.0f, cosf(yawRad), 0.0f);
	DirectX::XMVECTOR right = XMVectorSet(cosf(yawRad), 0.0f, -sinf(yawRad), 0.0f);

	if (Action::IsActionDown("MoveForward")) {
		direction = XMVectorAdd(direction, forward);
	}
	if (Action::IsActionDown("MoveBackward")) {
		direction = XMVectorAdd(direction, -forward);
	}
	if (Action::IsActionDown("MoveRight")) {
		direction = XMVectorAdd(direction, right);
	}
	if (Action::IsActionDown("MoveLeft")) {
		direction = XMVectorAdd(direction, -right);
	}
	if (Action::IsActionDown("MoveForward") && Action::IsActionDown("MoveRight")) {
		direction = XMVectorAdd(direction, forward);
		direction = XMVectorAdd(direction, right);
	}
	if (Action::IsActionDown("MoveRight") && Action::IsActionDown("MoveBackward")) {
		direction = XMVectorAdd(direction, right);
		direction = XMVectorAdd(direction, -forward);
	}
	if (Action::IsActionDown("MoveBackward") && Action::IsActionDown("MoveLeft")) {
		direction = XMVectorAdd(direction, -forward);
		direction = XMVectorAdd(direction, -right);
	}
	if (Action::IsActionDown("MoveLeft") && Action::IsActionDown("MoveForward")) {
		direction = XMVectorAdd(direction, -right);
		direction = XMVectorAdd(direction, forward);
	}

	// Gamepad left analog stick
	DirectX::XMFLOAT2 stick = Input::GetGamepadLeftStick();
	if (std::abs(stick.x) > 0.0f || std::abs(stick.y) > 0.0f)
	{
		direction = XMVectorAdd(direction, XMVectorScale(forward, stick.y));
		direction = XMVectorAdd(direction, XMVectorScale(right, stick.x));
	}

}

void PlayerControllerComponent::HandleMovement(float deltaTime)
{
    if (this->isPaused)
        return;

	auto transform = GetOwner()->GetComponent<TransformComponent>();
	float yawRad = XMConvertToRadians(this->yaw);

	DirectX::XMVECTOR forward = XMVectorSet(sinf(yawRad), 0.0f, cosf(yawRad), 0.0f);
	DirectX::XMVECTOR right = XMVectorSet(cosf(yawRad), 0.0f, -sinf(yawRad), 0.0f);

	DirectX::XMVECTOR moveDir = XMVectorZero();

	if (Input::IsGamepadConnected()) {
		DirectX::XMFLOAT2 stick = Input::GetGamepadLeftStick();
		if (std::abs(stick.x) > 0.0f || std::abs(stick.y) > 0.0f)
		{
			moveDir = XMVectorAdd(moveDir, XMVectorScale(forward, stick.y));
			moveDir = XMVectorAdd(moveDir, XMVectorScale(right, stick.x));
		}
	}

	if (Action::IsActionDown("MoveForward")) 
	{
		moveDir = XMVectorAdd(moveDir, forward);
	}
	if (Action::IsActionDown("MoveBackward")) 
	{
		moveDir = XMVectorSubtract(moveDir, forward);
	}
	if (Action::IsActionDown("MoveRight")) 
	{
		moveDir = XMVectorAdd(moveDir, right);
	}
	if (Action::IsActionDown("MoveLeft")) 
	{
		moveDir = XMVectorSubtract(moveDir, right);
	}

	if (!DirectX::XMVector3Equal(moveDir, XMVectorZero())) {
		moveDir = XMVector3Normalize(moveDir);

		XMFLOAT3 currentPos = transform->GetLocalPosition();
		XMVECTOR currentPosV = XMLoadFloat3(&currentPos);

        float moveSpeed = this->moveSpeed;
        if (this->isDoingHeavyAttack)
            moveSpeed = this->moveSpeed * 0.6f;

		XMVECTOR newVel = XMVectorScale(moveDir, deltaTime * moveSpeed);
		XMVECTOR newPosV = XMVectorAdd(currentPosV, newVel);

		float distMoved = XMVectorGetX(XMVector3Length(newVel));
		this->GetOwner()->GetComponent<CharacterAudioComponent>()->AddMovementDistance(distMoved);

		// Save the player velocity for arrow prediction (units per second)
		XMVECTOR actualVelocity = XMVectorScale(moveDir, moveSpeed);
		XMStoreFloat3(&this->velcoity, actualVelocity);

		XMFLOAT3 newPos;
		XMStoreFloat3(&newPos, newPosV);

		transform->SetLocalPosition(newPos);
	}
	else {
		this->velcoity = { 0.0f, 0.0f, 0.0f };
	}

    if (ViewBobComponent *viewBob = this->camera->GetOwner()->GetComponent<ViewBobComponent>())
        viewBob->SetMoveFactor(XMVectorGetX(XMVector3Length(XMLoadFloat3(&this->velcoity))) / this->moveSpeed);
}

void PlayerControllerComponent::HandleCameraLook(float deltaTime)
{
	if (Input::GetMouseMode() != MouseMode::CAPTURED)
		return;
	if (!this->canRotate)
		return;
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	float sensX = this->application ? this->application->GetMouseSensitivity() : this->mouseSensitivity.x;
	float sensY = this->application ? this->application->GetMouseSensitivity() : this->mouseSensitivity.y;
	float invertY = (this->application && this->application->GetInvertY()) ? -1.0f : 1.0f;
	float deltaX = Input::GetMouseDeltaX() * sensX;
	float deltaY = -Input::GetMouseDeltaY() * sensY * invertY;

	// Add Gamepad Right Stick input
	if (Input::IsGamepadConnected())
	{
		DirectX::XMFLOAT2 rightStick = Input::GetGamepadRightStick();
		float controllerSens = 120.0f; // degrees per second
		deltaX += rightStick.x * controllerSens * deltaTime;
		deltaY += rightStick.y * controllerSens * deltaTime * invertY; // Push up to look up
	}

	// Apply Horizontal (Yaw)
	this->yaw += deltaX;
	transform->SetLocalRotation(this->pitch, this->yaw, 0.0f);

	// Apply Vertical (Pitch)
	XMFLOAT3 rootRotation = this->rootTransform->GetLocalRotation();
	rootRotation.x += deltaY;
	rootRotation.x = std::clamp(rootRotation.x, -89.9f, 89.9f);
	this->rootTransform->SetLocalRotation(rootRotation);
}

void PlayerControllerComponent::HandleCombat(float deltaTime)
{
    if (this->isPaused)
        return;

	if (Action::IsActionPressed("LightAttack")) 
	{
		PerformLightAttack();
	}

	if (Action::IsActionPressed("HeavyAttack"))
	{
		PerformHeavyAttack();
	}

	if (Action::IsActionPressed("Dash") && this->canUseDash) 
	{
		this->dashDirection = DirectX::XMVectorZero();
		GetDashDirection(this->dashDirection);

		if (!DirectX::XMVector3Equal(this->dashDirection, DirectX::XMVectorZero())) {
			this->isDashing = true;
			this->canUseDash = false;
			this->dashTimer = 0.0f;

			DirectX::XMFLOAT4 dashColour = Debug::GetSetting("speedLines.colour", DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.0f });
			dashColour.w = this->maxDashAlpha;
			Debug::SetSetting("speedLines.colour", dashColour);

			Debug::SetSetting("speedLines.density", this->maxDashDensity);
			
			this->lingeringDashTimer = this->lingeringDashDuration;

			this->camera->SetFov(this->dashFov);

            this->audioManager->PlaySoundEffect(this->sfxDash, false);
		}
	}
}

void PlayerControllerComponent::PerformLightAttack()
{
	if (!this->CanAttack())
		return;
		
	if (auto *soundComponent = this->GetOwner()->GetComponent<SoundComponent>())
	{
		if (this->attackCounter % 3 == 0)
		{
			soundComponent->SetSound(this->attackShoutSound);
			soundComponent->Play();
		}
	}

	LogInfo("Attack with light attack.");

	this->currentAttack = this->attackCombo;

	this->PerformAttack(this->attacks[this->currentAttack]);

	this->attackCombo++;
	this->attackTimer = 0.0f;

	if (this->attackCombo >= this->numLightAttacks)
	{
		this->attackCombo = 0;
	}

	if (auto* characterAudio = this->GetOwner()->GetComponent<CharacterAudioComponent>())
	{
		characterAudio->PlayLightAttackSwing();
	}
}

void PlayerControllerComponent::PerformHeavyAttack()
{
	LogInfo("Attack with Heavy attack.");

	if (!this->CanAttack())
		return;

	this->currentAttack = 2; // Heavy attack has no combo

	this->PerformAttack(this->attacks[2]); // Heavy attack is attack 2

	this->attackCombo = 0;
	this->attackTimer = 0.0f;

	if (auto *characterAudio = this->GetOwner()->GetComponent<CharacterAudioComponent>())
	{
		characterAudio->PlayHeavyAttackSwing();
		characterAudio->PlayHeavyAttackShout();
	}

    this->isDoingHeavyAttack = true;
    this->heavyAttackTimer = 0.0f;
}

void PlayerControllerComponent::PerformAttack(AttackData &attackData)
{
	this->hasActivatedHitboxThisAttack = false;
	if (this->hitbox)
	{
		this->hitbox->Deactivate();
		this->hitbox->SetDamage(attackData.damage);
	}
	this->viewmodelAnimator->CrossFade(attackData.animationName, 0.1f, false);
	this->attackCounter++;
	this->attackCounterTimer = ATTACK_COUNTER_RESET_TIME;
}

bool PlayerControllerComponent::CanAttack()
{
	if (this->isParrying)
	{
		return false;
	}

	if (this->currentAttack == -1)
	{
		return true;
	}
	else
	{
		AttackData &currentAttackData = this->attacks[this->currentAttack];

		bool inAttackComboWindow =
			this->attackTimer >= currentAttackData.beginCombo &&
			this->attackTimer <= currentAttackData.endCombo;

		return inAttackComboWindow;
	}
}

void PlayerControllerComponent::HandleAttacks(float deltaTime)
{
    if (!this->isPaused) {
	    if (Action::IsActionDown("LightAttack"))
		    this->PerformLightAttack();

	    if (Action::IsActionPressed("Parry") && !this->isDoingHeavyAttack)
		    this->PerformParry();
    }

	if (this->isParrying)
	{
		if (this->viewmodelAnimator && this->viewmodelAnimator->IsFinished())
		{
			this->isParrying = false;
			this->viewmodelAnimator->CrossFade("idle", 0.2f);
		}
	}

	if (this->attackCounterTimer > 0.0f)
	{
		attackCounterTimer -= deltaTime;

		if (attackCounterTimer <= 0.0f)
		{
			this->attackCounter = 0;
		}
	}

	if (this->currentAttack != -1)
	{
		this->attackTimer += deltaTime;
		// LogInfo("Attack Timer: %f", this->attackTimer);
		AttackData &attackData = this->attacks[this->currentAttack];

		if (!this->hasActivatedHitboxThisAttack && this->attackTimer >= attackData.beginDamage)
		{
			this->hasActivatedHitboxThisAttack = true;
			if (this->hitbox)
			{
				this->hitbox->Activate(
					attackData.stopDamage - attackData.beginDamage
				);
			}
		}

		if (this->hitStopTimer >= 0.0)
		{
			this->viewmodelAnimator->SetSpeed(HIT_STOP_FACTOR);
			this->hitStopTimer -= deltaTime;
		}
		else
		{
			this->hitStopTimer = -1.0;
			this->viewmodelAnimator->SetSpeed(1.0f);
		}

		if (this->viewmodelAnimator->IsFinished())
		{
			this->currentAttack = -1;
			this->attackCombo = 0;
			this->hasActivatedHitboxThisAttack = false;
            this->viewmodelAnimator->SetSpeed(1.0f);
			if (this->hitbox)
			{
				this->hitbox->Deactivate();
			}

            this->viewmodelAnimator->CrossFade("idle", 0.2f);
		}
	}
}

void PlayerControllerComponent::PerformDash(float deltaTime, float dashDistance, float dashDuration)
{

	this->dashTimer += deltaTime;

	if (this->dashTimer >= dashDuration) {
		this->isDashing = false;
		this->velcoity = { 0.0f, 0.0f, 0.0f };
		return;
	}

	auto transform = GetOwner()->GetComponent<TransformComponent>();

	DirectX::XMVECTOR moveDir = DirectX::XMVector3Normalize(this->dashDirection);

	XMFLOAT3 currentPos = transform->GetLocalPosition();
	XMVECTOR currentPosV = XMLoadFloat3(&currentPos);

	float dashSpeed = dashDistance / dashDuration;

	XMVECTOR actualVel = XMVectorScale(moveDir, dashSpeed);
	XMStoreFloat3(&this->velcoity, actualVel);

	XMVECTOR newVel = XMVectorScale(moveDir, dashSpeed * deltaTime);
	XMVECTOR newPosV = XMVectorAdd(currentPosV, newVel);

	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos, newPosV);

	transform->SetLocalPosition(newPos);
}

void PlayerControllerComponent::PerformParry()
{
	if (this->isParrying)
		return;

	this->isParrying = true;
	this->currentAttack = -1;
	this->attackCombo = 0;
	this->hasActivatedHitboxThisAttack = false;
	if (this->hitbox)
	{
		this->hitbox->Deactivate();
	}

	if (this->viewmodelAnimator)
	{
		this->viewmodelAnimator->CrossFade("parry", 0.1f, false);
	}

	if (!this->hurtbox)
	{
		this->hurtbox = this->GetOwner()->GetComponent<HurtboxComponent>();
	}

	if (this->hurtbox)
	{
		this->hurtbox->GrantInvulnerability(this->parryWindow);
	}
}

void PlayerControllerComponent::OnAttackHit(const AttackHitEvent& event)
{
	if (!this->hitbox || event.attacker != this->hitbox->GetOwner())
		return;

	auto* characterAudio = this->GetOwner()->GetComponent<CharacterAudioComponent>();
	if (!characterAudio)
		return;

	if (this->currentAttack >= this->numLightAttacks)
	{
		characterAudio->PlayHeavyAttackHit();
	}
	else
	{
		characterAudio->PlayLightAttackHit();
	}

	if (this->hitStopTimer <= -1.0)
	{
		this->hitStopTimer = HIT_STOP_TIME;
	}

	IncrementCombo();
}

void PlayerControllerComponent::OnDamageTaken(const DamageTakenEvent& event)
{
	if (event.victim != this->GetOwner())
		return;

	if (this->health && this->health->IsDead())
	{
		this->OnDeath();
	}

	this->EndCombo();
}

void PlayerControllerComponent::OnHitBlocked(HitboxComponent* hitbox)
{
	if (!this->isParrying)
		return;

	this->parryCount++;
	this->parryCountResetTimer = PARRY_COUNTER_RESET_TIME;
	this->isParrying = false;

	uint32_t parryAnimIndex = 0;
	do
	{
		parryAnimIndex = static_cast<uint32_t>(RNG::Int(0, static_cast<int>(this->parryAnimations.size()) - 1));
	} while (parryAnimIndex == this->lastParryAnim);
	this->lastParryAnim = parryAnimIndex;

	const std::string &animName = this->parryAnimations[this->lastParryAnim];
	this->viewmodelAnimator->Play(animName, false);

	if (auto* parrySound = this->GetOwner()->GetComponent<AudioComponent>())
	{
		parrySound->SetAudio3D(false);
		parrySound->SetLooping(false);
		parrySound->SetPitch(RNG::Float(0.95f, 1.05f) * (1.0f + (this->parryCount - 1) * 0.3f));
		parrySound->Play();
	}

	if (auto *soundComponent = this->GetOwner()->GetComponent<SoundComponent>())
	{
		soundComponent->SetSound(this->parryShoutSound);
		soundComponent->Play();
	}

	if (this->camera)
	{
		if (auto* shake = this->camera->GetOwner()->GetComponent<ShakeComponent>())
			shake->AddTrauma(1.3f);
	}

	XMVECTOR lookVectorV = this->rootTransform->GetForwardV();

	// did we parry a projectile
	if (ProjectileComponent *projectile = hitbox->GetOwner()->GetComponent<ProjectileComponent>())
	{
		this->SpawnSpark(hitbox->GetOwner()->GetComponent<TransformComponent>()->GetWorldPosition());

		this->slowMotionTimer = SLOW_MOTION_TIME;
		if (this->audioManager)
			this->audioManager->SetPitchModifier(0.4f);

		hitbox->ClearIgnoreEntities();
		hitbox->SetDamage(hitbox->GetDamage() * 2);

		auto collision = hitbox->GetOwner()->GetComponent<CollisionComponent>();

		if (collision)
		{
			collision->SetLayer(CollisionLayer::PLAYER_ATTACK);
		}

		XMFLOAT3 velocityF = projectile->GetVelocity();
		XMVECTOR velocity = XMLoadFloat3(&velocityF);
		float length = XMVectorGetX(XMVector3Length(velocity));

		// viewmodel is rotated 180 degrees, hence why forward is scaled by minus here.
		XMVECTOR projectileVelocity = XMVectorScale(lookVectorV, -length * 2.0f); 
		XMFLOAT3 projectileVelocityF = {};
		XMStoreFloat3(&projectileVelocityF, projectileVelocity);

		auto transform = hitbox->GetOwner()->GetComponent<TransformComponent>();

		XMFLOAT3 projectilePositionF = rootTransform->GetWorldPosition();
		XMVECTOR projectilePosition = XMLoadFloat3(&projectilePositionF);
		projectilePosition = XMVectorAdd(projectilePosition, XMVectorScale(lookVectorV, -0.2f));
		projectilePosition = XMVectorAdd(projectilePosition, XMVectorSet(0, 0.22f, 0, 0));
		XMStoreFloat3(&projectilePositionF, projectilePosition);
		transform->SetWorldPosition(projectilePositionF);

		projectile->Launch(projectileVelocityF, { 0.0f, 0.0f, 0.0f });
	}
	// did we parry an enemy?
	else if (EnemyControllerComponent *enemyController = hitbox->GetOwner()->GetParent()->GetComponent<EnemyControllerComponent>())
	{
		hitbox->Deactivate();

		XMVECTOR knockbackDirectionV = -lookVectorV;
		knockbackDirectionV = XMVectorSetY(knockbackDirectionV, 0.0f);
		XMFLOAT3 knockbackDirection = {};
		XMStoreFloat3(&knockbackDirection, knockbackDirectionV);

		enemyController->ApplyKnockback(knockbackDirection, 4.0f);

		XMFLOAT3 sparkPositionF = rootTransform->GetWorldPosition();
		XMVECTOR sparkPosition = XMLoadFloat3(&sparkPositionF);
		sparkPosition = XMVectorAdd(sparkPosition, XMVectorScale(lookVectorV, -0.3f));
		sparkPosition = XMVectorAdd(sparkPosition, XMVectorSet(0, 0.3f, 0, 0));
		XMStoreFloat3(&sparkPositionF, sparkPosition);

		this->SpawnSpark(sparkPositionF);
	}

	this->onParry.Emit();
}

void PlayerControllerComponent::SpawnSpark(XMFLOAT3 position)
{
	// sparks
	{
		Entity *sparkEntity = this->GetOwner()->GetScene()->AddEntity("spark");
		sparkEntity->AddComponent<TransformComponent>()->SetWorldPosition(position);
		sparkEntity->AddComponent<LifeTimeComponent>()->SetLifeTime(1.0f);

		auto *particle = sparkEntity->AddComponent<ParticleSystemComponent>();
		particle->SetSpawnRadius(RNG::Float(0.1f, 0.3f));
		particle->SetIsBurst(true);
		particle->SetIsSpread(true);
		particle->SetCount(120);
		particle->SetStartTint({ 254.0f / 255.0f, 162.0f / 255.0f, 0.0f });
		particle->SetEndTint({ 0.0f, 0.0f, 0.0f });
		particle->SetStartScale(0.0f);
		particle->SetEndScale(1.5f);
		particle->SetLifeTime(1.0f);

		particle->SetVelocity({ 3.2f, 3.2f, 3.2f });
		particle->SetAcceleration({ 0.0f, -8.82f, 0.0f });

		particle->SetIsAnimate(true);
		particle->SetIsAdditive(true);
		particle->SetAtlasTexture(this->sparkTexture);
		particle->SetAnimationSpeed(10.0f);
		particle->SetAtlasSize({ 2.0f, 2.0f });
	}

	// shockwave
	{
		Entity *shockWaveEntity = this->GetOwner()->GetScene()->AddEntity("shockwave");
		shockWaveEntity->AddComponent<TransformComponent>()->SetWorldPosition(position);
		shockWaveEntity->AddComponent<LifeTimeComponent>()->SetLifeTime(0.08f);

		auto *particle = shockWaveEntity->AddComponent<ParticleSystemComponent>();
		particle->SetSpawnRadius(RNG::Float(0.0f, 0.0f));
		particle->SetIsBurst(true);
		particle->SetCount(1);
		particle->SetStartTint({ 254.0f * 0.5f / 255.0f, 162.0f * 0.5f / 255.0f, 0.0f });
		particle->SetEndTint({ 0.0f, 0.0f, 0.0f });
		particle->SetStartScale(0.5f);
		particle->SetEndScale(2.0f);
		particle->SetLifeTime(0.08f);
		particle->SetVelocity({ 0.0f, 0.0f, 0.0f });
		particle->SetAcceleration({ 0.0f, 0.0f, 0.0f });

		particle->SetIsAdditive(true);
		particle->SetAtlasTexture(this->parryShockwaveTexture);

		auto *pointLight = shockWaveEntity->AddComponent<PointLightComponent>();
		pointLight->SetColour({ 254.0f / 255.0f, 162.0f / 255.0f, 0.0f });
		pointLight->SetIntensity(150.0f);
		pointLight->SetRadius(5.0f);
	}

	IncrementCombo();
}

void PlayerControllerComponent::OnDeath()
{
	if (this->isDead)
		return;

	this->isDead = true;
	this->isParrying = false;

	if (this->hitbox)
		this->hitbox->Deactivate();


	if (auto* hurtbox = this->GetOwner()->GetComponent<HurtboxComponent>())
		hurtbox->SetActive(false);


	// TODO: Maybe enemies shouldn't assume that the player exists
	/*this->GetOwner()->GetScene()->RemoveEntityFromGroup(
		this->groupName, this->GetOwner()
	);*/

	DirectX::XMFLOAT4 dashColour = Debug::GetSetting("speedLines.colour", DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.0f });
	dashColour.w = 0.0f;
	Debug::SetSetting("speedLines.colour", dashColour);

	this->EndCombo();
	EventBus::Publish(PlayerDiedEvent{ this->GetOwner() });
}

bool PlayerControllerComponent::ConnectComboSignals()
{
	if (!this->enemySpawner)
	{
		LogError("Player Controller needs Enemy Spawner");
		return false;
	}

	if (!this->enemyDatabase)
	{
		LogError("Player Controller needs Enemy Databse");
		return false;
	}

	if (!this->waveController)
	{
		LogError("Player Controller needs Wave Controller");
		return false;
	}

	this->enemySpawner->GetOnEnemyKilled().Connect([this](EnemyType type, uint32_t wave)
	{
		auto enemy = this->enemyDatabase->GetEnemy(type);
		this->score += enemy->score;
		this->comboRemainingTime = this->comboDuration;
		this->onScoreChanged.Emit(this->score);

		if (this->waveController->IsFloorCompleted())
		{
			this->combo++;
			this->EndCombo();
		}
	});

	return true;
}

void PlayerControllerComponent::IncrementCombo()
{
	this->combo++;
	this->comboRemainingTime = this->comboDuration;
	this->onComboChanged.Emit(this->combo);
}

void PlayerControllerComponent::EndCombo()
{
	uint32_t resultScore = this->score * this->combo;
	uint32_t resultCombo = this->combo;

	this->score = 0;
	this->onScoreChanged.Emit(this->score);
	this->combo = 0;
	this->onComboChanged.Emit(this->combo);

	this->comboRemainingTime = -1.0f;
	
	this->onComboEnd.Emit(resultScore, resultCombo);
}

void PlayerControllerComponent::SetIsPaused(bool isPaused) {
    this->isPaused = isPaused;

    if (this->isPaused) {
        this->onPaused.Emit();
        Input::SetMouseMode(MouseMode::VISIBLE);
    }
    else {
        this->onUnpaused.Emit();
        Input::SetMouseMode(MouseMode::CAPTURED);
    }
}