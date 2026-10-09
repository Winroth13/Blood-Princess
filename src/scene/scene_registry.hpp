#pragma once

#include "scene/scene.hpp"
#include "core/app_context.hpp"

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>

class SceneRegistry {
    std::unordered_map<std::string, std::function<void(Scene &, const AppContext &)>> scenes;

public:
    SceneRegistry() = default;

    void AddScene(const std::string &name, std::function<void(Scene &, const AppContext &)> func);
    std::function<void(Scene &, const AppContext &)> *GetScene(const std::string &name);
    void Clear();

    std::vector<std::string> GetSceneNames() const;
};