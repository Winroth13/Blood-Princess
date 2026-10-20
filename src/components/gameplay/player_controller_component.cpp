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

#include "core/math.hpp"

#include <cmath>

void PlayerControllerComponent::OnStart(const AppContext& context)
{
	this->application = context.application;
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
			this->lingeringDashTimer = -1;
			DirectX::XMFLOAT4 dashColour = Debug::GetSetting("speedLines.colour", DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 0.0f });
			dashColour.w = 0.0f;
			Debug::SetSetting("speedLines.colour", dashColour);
			Debug::SetSetting("speedLines.density", this->minDashDensity);
			this->camera->SetFov(DirectX::XM_PIDIV2);
		}

		HandleMovement(deltaTime);
		HandleCombat(deltaTime); 
	}

	if (this->damageFlashLingeringTimer >= 0.0f)
	{
		float lingeringFactor = this->damageFlashLingeringTimer / this->damageFlashDuration;
		Debug::SetSetting("damageFlash.intensity", lingeringFactor);
		this->damageFlashLingeringTimer -= deltaTime;
	}
	else
	{
		this->damageFlashLingeringTimer = -1.0f;
		Debug::SetSetting("damageFlash.intensity", 0.0f);
	}

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
	else if (this->audioManager && this->audioManager->GetPitchModifier() != 1.0f)
	{
		this->audioManager->SetPitchModifier(1.0f);
	}

	if (this->comboRemainingTime != -1.0f)
	{
		if (this->comboRemainingTime < deltaTime)
		{
			this->comboRemainingTime = -1.0f;
			this->onComboRemainingTimeChanged.Emit(0.0f);
			this->onComboEnd.Emit(this->GetScore());
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
}

void PlayerControllerComponent::GetDashDirection(XMVECTOR &direction)
{
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
}

void PlayerControllerComponent::HandleMovement(float deltaTime)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	float yawRad = XMConvertToRadians(this->yaw);

	DirectX::XMVECTOR forward = XMVectorSet(sinf(yawRad), 0.0f, cosf(yawRad), 0.0f);
	DirectX::XMVECTOR right = XMVectorSet(cosf(yawRad), 0.0f, -sinf(yawRad), 0.0f);

	DirectX::XMVECTOR moveDir = XMVectorZero();


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

		XMVECTOR newVel = XMVectorScale(moveDir, deltaTime * this->moveSpeed);
		XMVECTOR newPosV = XMVectorAdd(currentPosV, newVel);

		float distMoved = XMVectorGetX(XMVector3Length(newVel));
		this->GetOwner()->GetComponent<CharacterAudioComponent>()->AddMovementDistance(distMoved);

		// Save the player velocity for arrow prediction (units per second)
		XMVECTOR actualVelocity = XMVectorScale(moveDir, this->moveSpeed);
		XMStoreFloat3(&this->velcoity, actualVelocity);

		XMFLOAT3 newPos;
		XMStoreFloat3(&newPos, newPosV);

		transform->SetLocalPosition(newPos);
	}
	else {
		this->velcoity = { 0.0f, 0.0f, 0.0f };
	}
}

void PlayerControllerComponent::HandleCameraLook(float deltaTime)
{
	if (Input::GetMouseMode() != MouseMode::CAPTURED)
		return;

	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	float deltaX = Input::GetMouseDeltaX();
	float deltaY = Input::GetMouseDeltaY();

	this->yaw += deltaX * this->mouseSensitivity.x;

	XMFLOAT3 rootRotation = this->rootTransform->GetLocalRotation();
	rootRotation.x += -deltaY * this->mouseSensitivity.y;
	rootRotation.x = std::clamp(rootRotation.x, -89.9f, 89.9f);
	this->rootTransform->SetLocalRotation(rootRotation);
	
	transform->SetLocalRotation(this->pitch, this->yaw, 0.0f);
	
}

void PlayerControllerComponent::HandleCombat(float deltaTime)
{
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
	if (Action::IsActionDown("LightAttack"))
		this->PerformLightAttack();

	if (Action::IsActionPressed("Parry"))
		this->PerformParry();

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
		this->viewmodelAnimator->Play("parry", false);
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
}

void PlayerControllerComponent::OnHitBlocked(HitboxComponent* hitbox)
{
	if (!this->isParrying)
		return;

	hitbox->Deactivate();

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

	XMFLOAT3 sparkPositionF = rootTransform->GetWorldPosition();
	XMVECTOR sparkPosition = XMLoadFloat3(&sparkPositionF);
	sparkPosition = XMVectorAdd(sparkPosition, XMVectorScale(lookVectorV, -0.3f));
	sparkPosition = XMVectorAdd(sparkPosition, XMVectorSet(0, 0.3f, 0, 0));
	XMStoreFloat3(&sparkPositionF, sparkPosition);
	this->SpawnSpark(hitbox->GetOwner()->GetComponent<TransformComponent>()->GetWorldPosition());

	// did we parry a projectile
	if (ProjectileComponent *projectile = hitbox->GetOwner()->GetComponent<ProjectileComponent>())
	{
		this->slowMotionTimer = SLOW_MOTION_TIME;
		if (this->audioManager)
			this->audioManager->SetPitchModifier(0.4f);

		hitbox->ClearIgnoreEntities();
		hitbox->SetDamage(hitbox->GetDamage() * 2);

		auto collision = hitbox->GetOwner()->GetComponent<BoxCollisionComponent>();
		collision->SetLayer(CollisionLayer::PLAYER_ATTACK);

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
		XMVECTOR knockbackDirectionV = -lookVectorV;
		knockbackDirectionV = XMVectorSetY(knockbackDirectionV, 0.0f);
		XMFLOAT3 knockbackDirection = {};
		XMStoreFloat3(&knockbackDirection, knockbackDirectionV);

		enemyController->ApplyKnockback(knockbackDirection, 8.0f);
	}
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
	Debug::SetSetting("damageFlash.intensity", 0.0f);

	if (this->hitbox)
		this->hitbox->Deactivate();


	if (auto* hurtbox = this->GetOwner()->GetComponent<HurtboxComponent>())
		hurtbox->SetActive(false);


	// TODO: Maybe enemies shouldn't assume that the player exists
	/*this->GetOwner()->GetScene()->RemoveEntityFromGroup(
		this->groupName, this->GetOwner()
	);*/

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

	this->enemySpawner->GetOnEnemyKilled().Connect([this](EnemyType type, uint32_t wave)
	{
		auto enemy = this->enemyDatabase->GetEnemy(type);
		this->score += enemy->score;
		this->comboRemainingTime = this->comboDuration;
		this->onScoreChanged.Emit(this->score);
	});

	return true;
}

void PlayerControllerComponent::IncrementCombo()
{
	this->combo++;
	this->comboRemainingTime = this->comboDuration;
	this->onComboChanged.Emit(this->combo);
}

uint32_t PlayerControllerComponent::ClaimScore()
{
	int resultScore = this->score * this->combo;
	this->score = 0;
	this->onScoreChanged.Emit(this->score);
	this->combo = 0;
	this->onComboChanged.Emit(this->combo);
	this->comboRemainingTime = -1.0f;
	return resultScore;
}