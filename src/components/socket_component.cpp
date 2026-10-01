#include "socket_component.hpp"
#include "components/transform_component.hpp"
#include "components/animator_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"

void SocketComponent::OnJointNameChanged() {
    this->resolvedSkeleton = nullptr;
    this->jointIndex = -1;
}

void SocketComponent::LateUpdate(const AppContext &context, float deltaTime) {
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    Entity *parent = this->GetOwner()->GetParent();
    if (!parent)
        return;

    AnimatorComponent *parentAnimator = parent->GetComponent<AnimatorComponent>();
    if (!parentAnimator)
        return;

    std::shared_ptr<Skeleton> skeleton = parentAnimator->GetSkeleton();
    if (!skeleton)
        return;

    if (skeleton != this->resolvedSkeleton) {
        this->resolvedSkeleton = skeleton;
        this->jointIndex = skeleton->FindJoint(this->jointName);

        if (this->jointIndex < 0)
            LogWarn("Joint not found :(");
    }

    const std::vector<XMFLOAT4X4> &jointMatrices = parentAnimator->GetJointModelMatrices();
    if (this->jointIndex < 0 || static_cast<size_t>(this->jointIndex) >= jointMatrices.size())
        return;

    XMMATRIX jointMatrix = XMLoadFloat4x4(&jointMatrices[this->jointIndex]);

    if (!this->shouldInheritScale) {
        XMVECTOR translation, rotation, scale;
        if (!XMMatrixDecompose(&scale, &rotation, &translation, jointMatrix))
            return;

        jointMatrix = XMMatrixRotationQuaternion(rotation) * XMMatrixTranslationFromVector(translation);
    }

    XMFLOAT3 pivot = {0.0f, 0.0f, 0.0f};
    if (TransformComponent *parentTransform = this->GetOwner()->GetParent()->GetComponent<TransformComponent>())
        pivot = parentTransform->GetLocalPivot();

    const XMMATRIX pivotMatrix = XMMatrixTranslation(-pivot.x, -pivot.y, -pivot.z);

    const XMFLOAT4 offsetQuaternion = TransformComponent::EulerAnglesToQuaternion(this->offsetRotation);
    const XMMATRIX offsetMatrix =
        XMMatrixScalingFromVector(XMLoadFloat3(&this->offsetScale)) *
        XMMatrixRotationQuaternion(XMLoadFloat4(&offsetQuaternion)) *
        XMMatrixTranslationFromVector(XMLoadFloat3(&this->offsetPosition)
    );

    transform->SetLocalMatrix(offsetMatrix * jointMatrix * pivotMatrix);
}
