#include "scene/component.hpp"

class ViewBobComponent : public Component
{
    RegisterComponent(ViewBobComponent);

    ExportPropertyCategory("Walk");
    ExportProperty(float, walkFrequency, 1.3f);
    ExportProperty(float, walkBobHeight, 0.1f);
    ExportProperty(float, walkBobSway, 0.025f);
    ExportProperty(float, walkRoll, 0.3f);

    ExportPropertyCategory("Idle");
    ExportProperty(float, idleFrequency, 0.25f);
    ExportProperty(float, idleBobHeight, 0.05f);

    ExportProperty(float, blendSpeed, 8.0f);

    float moveTarget = 0.0f;
    float moveBlend = 0.0f;
    float walkPhase = 0.0f;
    float idlePhase = 0.0f;

public:
    ViewBobComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

    void SetMoveFactor(float factor) { this->moveTarget = factor; }

    DirectX::XMFLOAT3 GetPositionOffset() const;
    DirectX::XMFLOAT3 GetRotationOffset() const;

private:
    void Update(const AppContext &context, float deltaTime) override;
};