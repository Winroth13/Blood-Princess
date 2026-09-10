#include "transform_component.hpp"
#include "scene/entity.hpp"

void TransformComponent::MarkLocalDirty() {
    this->isLocalDirty = true;
    this->MarkWorldDirty();
}

void TransformComponent::MarkWorldDirty() {
    if (this->isWorldDirty)
        return;

    this->isWorldDirty = true;

    Entity *owner = this->GetOwner();
    if (!owner)
        return;

    for (Entity *child : owner->GetChildren()) {
        TransformComponent *childTransform = child->GetComponent<TransformComponent>();
        if (childTransform)
            childTransform->MarkWorldDirty();
    }
}

XMFLOAT3 TransformComponent::ExtractScale(const XMMATRIX &matrix) {
    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, matrix);

    XMFLOAT3 output;
    XMStoreFloat3(&output, scale);
    return output;
}

XMFLOAT4 TransformComponent::ExtractRotation(const XMMATRIX &matrix) {
    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, matrix);

    XMFLOAT4 output;
    XMStoreFloat4(&output, rotation);
    return output;
}

XMFLOAT3 TransformComponent::ExtractTranslation(const XMMATRIX &matrix) {
    XMFLOAT4X4 m;
    XMStoreFloat4x4(&m, matrix);
    return {m._41, m._42, m._43};
}

// https://learn.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmmatrixrotationrollpitchyaw
// "The order of transformations is roll first, then pitch, and then yaw"
// [ cz * cy + sz * sx * sy,  sz * cx,  sz * sx * cy - cz * sy ]
// [ cz * sx * sy - sz * cy,  cz * cx,  sz * sy + cz * sx * cy ]
// [         cx * sy,          -sx,             cx * cy        ]
// where z = roll, x = pitch, y = yaw and s = sin, c = cos
XMFLOAT3 TransformComponent::QuaternionToEulerAngles(const XMFLOAT4 &quaternion) {
    XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion));
    XMFLOAT4X4 m;
    XMStoreFloat4x4(&m, rotationMatrix);

    XMFLOAT3 euler;

    float sinPitch = -m._32; // -(-sx) = sx
    sinPitch = sinPitch < -1.0f ? -1.0f : sinPitch > 1.0f ? 1.0f : sinPitch;

    if (fabsf(sinPitch) < 0.9999f) {
        euler.x = asinf(sinPitch);
        euler.y = atan2f(m._31, m._33); // (cx * sy) / (cx * cy) = sy / cy = ty
        euler.z = atan2f(m._12, m._22); // (sz * cx) / (cz * cx) = sz / cz = tz
    }
    else { // Gimbal lock
        // -(sz * sx * cy - cz * sy) / (cz * cy + sz * sx * sy), sx = 1 => 
        // (cz * sy - sz * cy) / (cz * cy + sz * sy) = s(z - y) / c(z - y) = t(z - y)
        // z = 0 => t(-y) = -ty
        // sx = -1 => t(-y) = -ty

        euler.x = sinPitch > 0.0f ? XM_PIDIV2 : -XM_PIDIV2;
        euler.y = atan2f(-m._13, m._11); 
        euler.z = 0.0f;
    }

    euler.x = XMConvertToDegrees(euler.x);
    euler.y = XMConvertToDegrees(euler.y);
    euler.z = XMConvertToDegrees(euler.z);

    return euler;
}

XMFLOAT4 TransformComponent::EulerAnglesToQuaternion(const XMFLOAT3 &euler) {
    XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(euler.x),
        XMConvertToRadians(euler.y),
        XMConvertToRadians(euler.z)
    );

    XMFLOAT4 output;
    XMStoreFloat4(&output, quaternion);

    return output;
}

void TransformComponent::SetLocalPosition(const XMFLOAT3 &position) {
    this->localPosition = position;
    this->MarkLocalDirty();
}

void TransformComponent::SetLocalPosition(float x, float y, float z) {
    this->SetLocalPosition({x, y, z});
}

XMFLOAT3 TransformComponent::GetLocalPosition() const {
    return this->localPosition;
}

void TransformComponent::SetLocalRotation(const XMFLOAT3 &rotation) {
    this->localRotation = this->EulerAnglesToQuaternion(rotation);
    this->localEuler = rotation;

    this->MarkLocalDirty();
}

void TransformComponent::SetLocalRotation(float x, float y, float z) {
    this->SetLocalRotation({x, y, z});
}

XMFLOAT3 TransformComponent::GetLocalRotation() const {
    return this->localEuler;
}

void TransformComponent::SetLocalRotationQuaternion(const XMFLOAT4 &quaternion) {
    XMStoreFloat4(&this->localRotation, XMQuaternionNormalize(XMLoadFloat4(&quaternion)));
    this->localEuler = this->QuaternionToEulerAngles(this->localRotation);
    this->MarkLocalDirty();
}

XMFLOAT4 TransformComponent::GetLocalRotationQuaternion() const {
    return this->localRotation;
}

void TransformComponent::SetLocalScale(const XMFLOAT3 &scale) {
    this->localScale = scale;
    this->MarkLocalDirty();
}

void TransformComponent::SetLocalScale(float x, float y, float z) {
    this->SetLocalScale({x, y, z});
}

void TransformComponent::SetLocalScale(float scale) {
    this->SetLocalScale({scale, scale, scale});
}

XMFLOAT3 TransformComponent::GetLocalScale() const {
    return this->localScale;
}

void TransformComponent::SetLocalPivot(const XMFLOAT3 &pivot) {
    this->localPivot = pivot;
}

XMFLOAT3 TransformComponent::GetLocalPivot() const {
    return this->localPivot;
}

void TransformComponent::SetWorldPosition(const XMFLOAT3 &position) {
    Entity *owner = this->GetOwner();
    Entity *parent = owner ? owner->GetParent() : nullptr;

    if (!parent) {
        this->SetLocalPosition(position);
        return;
    }

    TransformComponent *parentTransform = parent->GetComponent<TransformComponent>();
    if (!parentTransform) {
        this->SetLocalPosition(position);
        return;
    }

    XMMATRIX invParentWorldMatrix = XMMatrixInverse(nullptr, parentTransform->GetWorldMatrix());
    XMVECTOR localPosition = XMVector3Transform(XMLoadFloat3(&position), invParentWorldMatrix);

    XMFLOAT3 result;
    XMStoreFloat3(&result, localPosition);
    this->SetLocalPosition(result);
}

void TransformComponent::SetWorldPosition(float x, float y, float z) {
    this->SetWorldPosition({x, y, z});
}

XMFLOAT3 TransformComponent::GetWorldPosition() const {
    return this->ExtractTranslation(this->GetWorldMatrix());
}

void TransformComponent::SetWorldRotation(const XMFLOAT3 &rotation) {
    XMVECTOR quaternion = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(rotation.x),
        XMConvertToRadians(rotation.y),
        XMConvertToRadians(rotation.z)
    );

    XMFLOAT4 output;
    XMStoreFloat4(&output, quaternion);
    this->SetWorldRotationQuaternion(output);
}

void TransformComponent::SetWorldRotation(float x, float y, float z) {
    this->SetWorldRotation({x, y, z});
}

XMFLOAT3 TransformComponent::GetWorldRotation() const {
    XMFLOAT4 quaternion = this->GetWorldRotationQuaternion();
    return this->QuaternionToEulerAngles(quaternion);
}

void TransformComponent::SetWorldRotationQuaternion(const XMFLOAT4 &quaternion) {
    Entity *owner = this->GetOwner();
    Entity *parent = owner ? owner->GetParent() : nullptr;

    if (!parent) {
        this->SetLocalRotationQuaternion(quaternion);
        return;
    }

    TransformComponent *parentTransform = parent->GetComponent<TransformComponent>();
    if (!parentTransform) {
        this->SetLocalRotationQuaternion(quaternion);
        return;
    }

    XMFLOAT4 q = parentTransform->GetWorldRotationQuaternion();
    XMVECTOR parentRotation = XMLoadFloat4(&q);
    XMVECTOR invParentRotation = XMQuaternionInverse(parentRotation);
    XMVECTOR localRotation = XMQuaternionMultiply(XMLoadFloat4(&quaternion), invParentRotation);

    XMFLOAT4 result;
    XMStoreFloat4(&result, localRotation);
    this->SetLocalRotationQuaternion(result);
}

XMFLOAT4 TransformComponent::GetWorldRotationQuaternion() const {
    return this->ExtractRotation(this->GetWorldMatrix());
}

void TransformComponent::SetWorldScale(const XMFLOAT3 &scale) {
    Entity *owner = this->GetOwner();
    Entity *parent = owner ? owner->GetParent() : nullptr;

    if (!parent) {
        this->SetLocalScale(scale);
        return;
    }

    TransformComponent *parentTransform = parent->GetComponent<TransformComponent>();
    if (!parentTransform) {
        this->SetLocalScale(scale);
        return;
    }

    XMMATRIX parentWorld = parentTransform->GetWorldMatrix();
    XMVECTOR parentScale, parentRotation, parentTranslation;
    XMMatrixDecompose(&parentScale, &parentRotation, &parentTranslation, parentWorld);

    XMVECTOR localScale = XMVectorDivide(XMLoadFloat3(&scale), parentScale);

    XMFLOAT3 result;
    XMStoreFloat3(&result, localScale);
    this->SetLocalScale(result);
}

void TransformComponent::SetWorldScale(float x, float y, float z) {
    this->SetWorldScale({x, y, z});
}

void TransformComponent::SetWorldScale(float scale) {
    this->SetWorldScale({scale, scale, scale});
}

XMFLOAT3 TransformComponent::GetWorldScale() const {
    return this->ExtractScale(this->GetWorldMatrix());
}

XMFLOAT3 TransformComponent::GetRenderPosition() const {
    return this->ExtractTranslation(this->GetRenderMatrix());
}

XMMATRIX TransformComponent::GetLocalMatrix() const {
    if (this->isLocalDirty) {
        XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&this->localScale));
        XMMATRIX rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&this->localRotation));
        XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&this->localPosition));

        XMStoreFloat4x4(&this->cachedLocal, scale * rotation * translation);
        this->isLocalDirty = false;
    }

    return XMLoadFloat4x4(&this->cachedLocal);
}

void TransformComponent::SetWorldMatrix(const XMMATRIX &worldMatrix) {
    XMMATRIX localMatrix = worldMatrix;

    Entity *owner = this->GetOwner();
    Entity *parent = owner ? owner->GetParent() : nullptr;

    if (parent) {
        TransformComponent *parentTransform = parent->GetComponent<TransformComponent>();
        if (parentTransform) {
            XMMATRIX invParentWorldMatrix = XMMatrixInverse(nullptr, parentTransform->GetWorldMatrix());
            localMatrix = worldMatrix * invParentWorldMatrix;
        }
    }

    XMVECTOR scale, rotation, translation;
    if (!XMMatrixDecompose(&scale, &rotation, &translation, localMatrix))
        return;

    XMStoreFloat3(&this->localScale, scale);

    XMStoreFloat4(&this->localRotation, rotation);
    this->localEuler = this->QuaternionToEulerAngles(this->localRotation);

    XMStoreFloat3(&this->localPosition, translation);

    this->MarkLocalDirty();
}

XMMATRIX TransformComponent::GetWorldMatrix() const {
    if (!this->isWorldDirty)
        return XMLoadFloat4x4(&this->cachedWorld);

    XMMATRIX localMatrix = this->GetLocalMatrix();
    XMMATRIX worldMatrix = localMatrix;

    Entity *owner = this->GetOwner();
    Entity *parent = owner ? owner->GetParent() : nullptr;

    if (parent) {
        TransformComponent *parentTransform = parent->GetComponent<TransformComponent>();
        if (parentTransform)
            worldMatrix = localMatrix * parentTransform->GetWorldMatrix();
    }

    XMStoreFloat4x4(&this->cachedWorld, worldMatrix);
    this->isWorldDirty = false;

    return XMLoadFloat4x4(&this->cachedWorld);
}

XMMATRIX TransformComponent::GetRenderMatrix() const {
    XMMATRIX pivotMatrix = XMMatrixTranslationFromVector(XMVectorNegate(XMLoadFloat3(&this->localPivot)));
    return pivotMatrix * this->GetWorldMatrix();
}

XMFLOAT3 TransformComponent::GetForward() const {
    XMFLOAT3 output;
    XMStoreFloat3(&output, this->GetForwardV());
    return output;
}

XMVECTOR TransformComponent::GetForwardV() const {
    XMMATRIX worldMatrix = this->GetWorldMatrix();
    return XMVector3Normalize(worldMatrix.r[2]);
}

XMFLOAT3 TransformComponent::GetRight() const {
    XMFLOAT3 output;
    XMStoreFloat3(&output, this->GetRightV());
    return output;
}

XMVECTOR TransformComponent::GetRightV() const {
    XMMATRIX worldMatrix = this->GetWorldMatrix();
    return XMVector3Normalize(worldMatrix.r[0]);
}

XMFLOAT3 TransformComponent::GetUp() const {
    XMFLOAT3 output;
    XMStoreFloat3(&output, this->GetUpV());
    return output;
}

XMVECTOR TransformComponent::GetUpV() const {
    XMMATRIX worldMatrix = this->GetWorldMatrix();
    return XMVector3Normalize(worldMatrix.r[1]);
}

void TransformComponent::LookAt(const XMFLOAT3 &worldTarget, const XMFLOAT3 &up) {
    XMFLOAT3 worldPosition = this->GetWorldPosition();

    XMVECTOR eye = XMLoadFloat3(&worldPosition);
    XMVECTOR target = XMLoadFloat3(&worldTarget);
    XMVECTOR upVector = XMLoadFloat3(&up);

    if (XMVector3NearEqual(eye, target, XMVectorReplicate(1e-5f)))
        return;

    XMMATRIX viewMatrix = XMMatrixLookAtLH(eye, target, upVector);
    XMMATRIX worldMatrix = XMMatrixInverse(nullptr, viewMatrix);

    XMFLOAT4 rotation = this->ExtractRotation(worldMatrix);
    this->SetWorldRotationQuaternion(rotation);
}

XMFLOAT3 TransformComponent::TransformPoint(const XMFLOAT3 &localPoint) const {
    XMVECTOR worldPoint = XMVector3Transform(XMLoadFloat3(&localPoint), this->GetWorldMatrix());
    XMFLOAT3 output;
    XMStoreFloat3(&output, worldPoint);
    return output;
}

XMFLOAT3 TransformComponent::TransformPointRender(const XMFLOAT3 &localPoint) const {
    XMVECTOR worldPoint = XMVector3Transform(XMLoadFloat3(&localPoint), this->GetRenderMatrix());
    XMFLOAT3 output;
    XMStoreFloat3(&output, worldPoint);
    return output;
}

XMFLOAT3 TransformComponent::TransformDirection(const XMFLOAT3 &localDirection) const {
    XMVECTOR worldDir = XMVector3TransformNormal(XMLoadFloat3(&localDirection), this->GetWorldMatrix());
    XMFLOAT3 output;
    XMStoreFloat3(&output, worldDir);
    return output;
}

XMFLOAT3 TransformComponent::InverseTransformPoint(const XMFLOAT3 &worldPoint) const {
    XMMATRIX inverse = XMMatrixInverse(nullptr, this->GetWorldMatrix());
    XMVECTOR localPoint = XMVector3Transform(XMLoadFloat3(&worldPoint), inverse);
    XMFLOAT3 output;
    XMStoreFloat3(&output, localPoint);
    return output;
}

XMFLOAT3 TransformComponent::InverseTransformPointRender(const XMFLOAT3 &worldPoint) const {
    XMMATRIX inverse = XMMatrixInverse(nullptr, this->GetRenderMatrix());
    XMVECTOR localPoint = XMVector3Transform(XMLoadFloat3(&worldPoint), inverse);
    XMFLOAT3 output;
    XMStoreFloat3(&output, localPoint);
    return output;
}

XMFLOAT3 TransformComponent::InverseTransformDirection(const XMFLOAT3 &worldDirection) const {
    XMMATRIX inverse = XMMatrixInverse(nullptr, this->GetWorldMatrix());
    XMVECTOR localDir = XMVector3TransformNormal(XMLoadFloat3(&worldDirection), inverse);
    XMFLOAT3 output;
    XMStoreFloat3(&output, localDir);
    return output;
}
