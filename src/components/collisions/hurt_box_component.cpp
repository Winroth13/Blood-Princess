#include "hurt_box_component.hpp"
#include "collision/collision_component.hpp"
#include "components/core/transform_component.hpp"


void HurtboxComponent::OnStart(const AppContext& context)
{
	this->cachedHealth = this->GetOwner()->GetComponent<Health>();

    this->collisionEventConn = EventBus::Subscribe<CollisionEvent>(
        this, &HurtboxComponent::OnCollision
    );
}

void HurtboxComponent::Update(const AppContext& context, float deltaTime)
{
    if (this->invulnerabilityTimer > 0.0f)
    {
        this->invulnerabilityTimer -= deltaTime;
        if (this->invulnerabilityTimer <= 0.0f)
        {
            this->invulnerabilityTimer = 0.0f;
            this->isInvulnerable = false;
        }
    }
}

void HurtboxComponent::OnDestroy(const AppContext& context)
{
    if (this->collisionEventConn != 0) {
        EventBus::Unsubscribe<CollisionEvent>(this->collisionEventConn);
        this->collisionEventConn = 0;
    }
}

void HurtboxComponent::OnCollision(const CollisionEvent& e)
{

    if (!e.selfCollider || e.selfCollider->GetOwner() != this->GetOwner())
        return;

    if (!this->IsActive() || !e.otherCollider)
        return;

    Entity* otherEntity = e.otherCollider->GetOwner();
    if (!otherEntity || !otherEntity->IsActive())
        return;


    HitboxComponent* hitbox = otherEntity->GetComponent<HitboxComponent>();
    if (!hitbox || !hitbox->IsActive())
        return;

    Entity* victim = this->GetOwner();
    if (!hitbox->CanHit(victim))
        return;

    if (this->IsInvulnerable())
    {
        this->onHitBlocked.Emit(hitbox);
        return;
    }

    TransformComponent* victimTransform = victim->GetComponent<TransformComponent>();
    DirectX::XMFLOAT3 hitPos = victimTransform ? victimTransform->GetWorldPosition() : DirectX::XMFLOAT3(0, 0, 0);
    float finalDamage = hitbox->GetDamage() * this->damageMultiplier;


    if (!this->cachedHealth)
        this->cachedHealth = victim->GetComponent<Health>();
    if (this->cachedHealth)
    {
        this->cachedHealth->TakeDamage(finalDamage);
    }

    hitbox->OnSuccessfulHit(victim, hitPos);

    EventBus::Publish(DamageTakenEvent{
        victim,
        hitbox->GetOwner(),
        finalDamage,
        hitPos,
        e.normal
        });



    LogInfo("The HurtBox is lossing Hp, Max: %f, Current: %f", this->cachedHealth->GetMaxHealth(), this->cachedHealth->GetCurrentHealth());
}


bool HurtboxComponent::ReceiveHit(HitboxComponent* hitbox, const DirectX::XMFLOAT3& hitPoint, const DirectX::XMFLOAT3& hitNormal)
{
    if (!this->IsActive() || !hitbox)
        return false;

    Entity* target = this->GetOwner();
    if (!hitbox->CanHit(target))
        return false;

    if (this->IsInvulnerable())
    {
        this->onHitBlocked.Emit(hitbox);
        return false;
    }

    float finalDamage = hitbox->GetDamage() * this->damageMultiplier;

    
    if (!this->cachedHealth)
        this->cachedHealth = target->GetComponent<Health>();

    if (this->cachedHealth)
    {
        this->cachedHealth->TakeDamage(finalDamage);
    }

    hitbox->OnSuccessfulHit(target, hitPoint);

    EventBus::Publish(DamageTakenEvent{
        target,
        hitbox->GetOwner(),
        finalDamage,
        hitPoint,
        hitNormal
        });

    return true;
}

void HurtboxComponent::GrantInvulnerability(float duration)
{
    this->isInvulnerable = true;
    this->invulnerabilityTimer = duration;
}
