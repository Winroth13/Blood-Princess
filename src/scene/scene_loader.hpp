#pragma once

#include "scene/scene.hpp"

#include <string>

struct AppContext;

class SceneLoader {
    const AppContext* appContext;

public:
    SceneLoader() = default;

    bool Initialize(const AppContext &appContext);

    bool Load(const std::string &name, Scene &outScene);
};