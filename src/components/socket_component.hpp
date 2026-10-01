#pragma once

#include "scene/entity.hpp"
#include "scene/component.hpp"

#include <memory>

class Skeleton;

class SocketComponent : public Component {
    RegisterComponent(SocketComponent);

    ExportProperty(std::string, jointName, "");
    ExportPropertyChanged(jointName, OnJointNameChanged);

    ExportProperty(bool, shouldInheritScale, false);

    ExportPropertyCategory("Offset");
    ExportProperty(DirectX::XMFLOAT3, offsetPosition, {0.0f, 0.0f, 0.0f});
    ExportProperty(DirectX::XMFLOAT3, offsetRotation, {0.0f, 0.0f, 0.0f});
    ExportProperty(DirectX::XMFLOAT3, offsetScale, {1.0f, 1.0f, 1.0f});

    std::shared_ptr<Skeleton> resolvedSkeleton = nullptr;
    int32_t jointIndex = -1;

    void OnJointNameChanged();

public:
    SocketComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoRender(); }

private:
    void LateUpdate(const AppContext &context, float deltaTime) override;

public:
    const std::string &GetJointName() const { return this->jointName; }
    void SetJointName(const std::string &jointName) { this->jointName = jointName; }

    bool GetShouldInheritScale() const { return this->shouldInheritScale; }
    void SetShouldInheritScale(bool shouldInheritScale) { this->shouldInheritScale = shouldInheritScale; }

    DirectX::XMFLOAT3 GetOffsetPosition() const { return this->offsetPosition; }
    void SetOffsetPosition(const DirectX::XMFLOAT3 &offsetPosition) { this->offsetPosition = offsetPosition; }

    DirectX::XMFLOAT3 GetOffsetRotation() const { return this->offsetRotation; }
    void SetOffsetRotation(const DirectX::XMFLOAT3 &offsetRotation) { this->offsetRotation = offsetRotation; }

    DirectX::XMFLOAT3 GetOffsetScale() const { return this->offsetScale; }
    void SetOffsetScale(const DirectX::XMFLOAT3 &offsetScale) { this->offsetScale = offsetScale; }

    int32_t GetJointIndex() const { return this->jointIndex; }
};