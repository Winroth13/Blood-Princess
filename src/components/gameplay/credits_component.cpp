#include "credits_component.hpp"
#include "scene/entity.hpp"
#include "components/ui/ui_rect_component.hpp"

void CreditsComponent::OnStart(const AppContext &context)
{
	UIRectComponent *rectComp = this->GetOwner()->GetComponent<UIRectComponent>();
	this->creditsHeight = rectComp->GetSizeDelta().y;
	rectComp->SetPosition({ 0.0f, this->bottomHeight });
}

void CreditsComponent::Update(const AppContext &context, float deltaTime)
{
	if (this->isFirstFrame)
	{
		this->isFirstFrame = false;
		return;
	}

	UIRectComponent *rectComp = this->GetOwner()->GetComponent<UIRectComponent>();
	float currentHeight = rectComp->GetPosition().y;

	float resultScrollSpeed = this->scrollSpeed;
	if ((Input::IsKeyDown(Key::SPACE) || Input::IsGamepadButtonDown(GamepadButton::SOUTH)))
	{
		resultScrollSpeed *= this->fastScrollMult;
	}

	if (currentHeight < this->topHeight - this->creditsHeight)
		rectComp->SetPosition({ 0.0f, this->bottomHeight });
	else
		rectComp->SetPosition({ 0.0f, currentHeight - deltaTime * resultScrollSpeed });
}
