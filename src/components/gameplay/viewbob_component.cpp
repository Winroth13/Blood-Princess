#include "viewbob_component.hpp"

void ViewBobComponent::Update(const AppContext &context, float deltaTime) {
    this->moveBlend += (this->moveTarget - this->moveBlend) * (1.0f - std::exp(-this->blendSpeed * deltaTime));

    this->walkPhase += deltaTime * this->walkFrequency * this->moveBlend * XM_2PI;
    this->idlePhase += deltaTime * this->idleFrequency * XM_2PI;
}

XMFLOAT3 ViewBobComponent::GetPositionOffset() const {
    float walkY = (std::abs(std::sin(this->walkPhase)) - 1.0f) * this->walkBobHeight;
    float walkX = std::cos(this->walkPhase) * this->walkBobSway;
    float idleY = std::sin(this->idlePhase) * this->idleBobHeight;

    return {
        walkX * this->moveBlend,
        walkY * this->moveBlend + idleY * (1.0f - this->moveBlend),
        0.0f
    };
}

XMFLOAT3 ViewBobComponent::GetRotationOffset() const {
    return { 0.0f, 0.0f, std::cos(this->walkPhase) * this->walkRoll * this->moveBlend };
}