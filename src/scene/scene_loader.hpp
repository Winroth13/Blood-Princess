#pragma once

#include "scene/scene.hpp"

#include <string>

class SceneLoader {
public:
    SceneLoader() = default;

    bool Load(const std::string &name, Scene &outScene);
};