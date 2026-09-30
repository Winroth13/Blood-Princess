#pragma once

#include "scene/entity.hpp"
#include "scene/component.hpp"

#include <memory>

class Skeleton;

class SocketComponent : public Component {
    RegisterComponent(SocketComponent);

    //ExportProperty(std::string, jointName, "");
    std::string jointName = "";
    // ExportPropertyChanged()

    ExportProperty(bool, shouldInheritScale, false);

    ExportPropertyCategory("Offset");
    ExportProperty(DirectX::XMFLOAT3, offsetPosition, {0.0f, 0.0f, 0.0f});
    ExportProperty(DirectX::XMFLOAT3, offsetRotation, {0.0f, 0.0f, 0.0f});
    ExportProperty(DirectX::XMFLOAT3, offsetScale, {1.0f, 1.0f, 1.0f});

    std::shared_ptr<Skeleton> resolvedSkeleton = nullptr;
    int32_t jointIndex = -1;

public:
    SocketComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoRender(); }

private:
    void LateUpdate(const AppContext &context, float deltaTime) override;
};