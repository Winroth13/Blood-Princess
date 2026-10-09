#include "scene_registry.hpp"

void SceneRegistry::AddScene(const std::string &name, std::function<void(Scene &, const AppContext &)> func) { 
    this->scenes[name] = func; 
}

std::function<void(Scene &, const AppContext &)> *SceneRegistry::GetScene(const std::string &name) {
    auto iter = this->scenes.find(name);
    if (iter == this->scenes.end())
        return nullptr;

    return &iter->second;
}

void SceneRegistry::Clear() { 
    this->scenes.clear();
}

std::vector<std::string> SceneRegistry::GetSceneNames() const {
    std::vector<std::string> result;
    result.reserve(this->scenes.size());

    for (auto &[name, func] : this->scenes)
        result.push_back(name);

    return result;
}
