#pragma once

#include "rendering/render_view.hpp"

#include <vector>

class Component;

class SceneCuller {
    std::vector<Component *> cullable;
    std::vector<Component *> alwaysVisible;

public:
    SceneCuller() = default;
    ~SceneCuller() = default;

    SceneCuller(const SceneCuller &other) = delete;
    SceneCuller &operator=(const SceneCuller &other) = delete;

    void AddComponent(Component *component);
    void RemoveComponent(Component *component);

    void Clear();

    void GatherRenderCommands(std::vector<RenderView> &outViews) const;
};