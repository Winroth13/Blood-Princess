#pragma once

#include "scene/scene.hpp"

#include <DirectXMath.h>
#include <string>

struct AppContext;
class SceneRegistry;

class SceneLoader {
    const AppContext* appContext;
    SceneRegistry *registry = nullptr;

public:
    SceneLoader() = default;

    bool Initialize(const AppContext &appContext, SceneRegistry *registry);

    bool Load(const std::string &name, Scene &outScene);
};