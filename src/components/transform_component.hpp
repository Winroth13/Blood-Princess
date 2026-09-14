#pragma once
#include "scene/component.hpp"

#include <DirectXMath.h>

using namespace DirectX;

class TransformComponent : public Component
{
    RegisterComponent(TransformComponent)

    ExportPropertyColor(XMFLOAT3, localPosition, { 0.0f, 0.0f, 0.0f })
    OnPropertyChanged(localPosition, MarkLocalDirty);

    ExportProperty(XMFLOAT3, localScale , {1.0f, 1.0f, 1.0f})
    OnPropertyChanged(localScale, MarkLocalDirty);

    ExportProperty(XMFLOAT3, localPivot, {0.0f, 0.0f, 0.0f})

    ExportProperty(XMFLOAT3, localEuler, {0.0f, 0.0f, 0.0f})
    OnPropertyChanged(localEuler, OnLocalEulerEdited);

    ExportProperty(XMFLOAT4, localRotation, { 0.0f, 0.0f, 0.0f, 1.0f }) // Quaternion
    OnPropertyChanged(localRotation, OnLocalRotationEdited);

    mutable XMFLOAT4X4 cachedLocal{};
    mutable bool isLocalDirty = true;

    mutable XMFLOAT4X4 cachedWorld{};
    mutable bool isWorldDirty = true;

    void MarkLocalDirty();
    void MarkWorldDirty();

    // Keeps localEuler and localRotation in sync when either one is edited directly.
    void OnLocalEulerEdited();
    void OnLocalRotationEdited();

public:
    static XMFLOAT3 ExtractScale(const XMMATRIX &matrix);
    static XMFLOAT4 ExtractRotation(const XMMATRIX &matrix);
    static XMFLOAT3 ExtractTranslation(const XMMATRIX &matrix);

    static XMFLOAT3 QuaternionToEulerAngles(const XMFLOAT4 &quaternion);
    static XMFLOAT4 EulerAnglesToQuaternion(const XMFLOAT3 &euler);

    TransformComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoRender(); }

    void OnStart(const AppContext &context) override {}
    void Update(const AppContext &context, float deltaTime) override {}
    void Render(const RenderView &view, RenderQueue &queue) override {}
    void OnDestroy(const AppContext &context) override {}

    // TODO: Add XMVECTOR versions

    void SetLocalPosition(const XMFLOAT3 &position);
    void SetLocalPosition(float x, float y, float z);
    XMFLOAT3 GetLocalPosition() const;

    // Euler angles, in degrees
    void SetLocalRotation(const XMFLOAT3 &rotation);
    void SetLocalRotation(float x, float y, float z);
    XMFLOAT3 GetLocalRotation() const;

    void SetLocalRotationQuaternion(const XMFLOAT4 &quaternion);
    XMFLOAT4 GetLocalRotationQuaternion() const;

    void SetLocalScale(const XMFLOAT3 &scale);
    void SetLocalScale(float x, float y, float z);
    void SetLocalScale(float scale);
    XMFLOAT3 GetLocalScale() const;

    void SetLocalPivot(const XMFLOAT3 &pivot);
    XMFLOAT3 GetLocalPivot() const;

    void SetWorldPosition(const XMFLOAT3 &position);
    void SetWorldPosition(float x, float y, float z);
    XMFLOAT3 GetWorldPosition() const;

    // Euler angles, in degrees
    void SetWorldRotation(const XMFLOAT3 &rotation);
    void SetWorldRotation(float x, float y, float z);
    XMFLOAT3 GetWorldRotation() const;

    void SetWorldRotationQuaternion(const XMFLOAT4 &quaternion);
    XMFLOAT4 GetWorldRotationQuaternion() const;

    void SetWorldScale(const XMFLOAT3 &scale);
    void SetWorldScale(float x, float y, float z);
    void SetWorldScale(float scale);
    XMFLOAT3 GetWorldScale() const;

    XMFLOAT3 GetRenderPosition() const;

    XMMATRIX GetLocalMatrix() const;

    void SetWorldMatrix(const XMMATRIX &worldMatrix);
    XMMATRIX GetWorldMatrix() const;

    // World matrix relative to pivot
    XMMATRIX GetRenderMatrix() const;

    XMFLOAT3 GetForward() const;
    XMVECTOR GetForwardV() const;
    XMFLOAT3 GetRight() const;
    XMVECTOR GetRightV() const;
    XMFLOAT3 GetUp() const;
    XMVECTOR GetUpV() const;

    void LookAt(const XMFLOAT3 &worldTarget, const XMFLOAT3 &up = {0.0f, 1.0f, 0.0});

    // Local space -> World space
    XMFLOAT3 TransformPoint(const XMFLOAT3 &localPoint) const;
    XMFLOAT3 TransformPointRender(const XMFLOAT3 &localPoint) const;
    XMFLOAT3 TransformDirection(const XMFLOAT3 &localDirection) const;

    // World space -> Local space
    XMFLOAT3 InverseTransformPoint(const XMFLOAT3 &worldPoint) const;
    XMFLOAT3 InverseTransformPointRender(const XMFLOAT3 &worldPoint) const;
    XMFLOAT3 InverseTransformDirection(const XMFLOAT3 &worldDirection) const;

    bool IsWorldDirty() const { return this->isWorldDirty; }
};