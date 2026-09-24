#pragma once

#include "scene/scene.hpp"

#include <DirectXMath.h>
#include <string>

struct AppContext;

class SceneLoader {
    const AppContext* appContext;

public:
    SceneLoader() = default;

    bool Initialize(const AppContext &appContext);

    bool Load(const std::string &name, Scene &outScene);

    static void CreateCircularRoom(
        Scene &outScene, 
        const AppContext &appContext, 
        const DirectX::XMFLOAT3 &center = { 0.0f, 0.0f, 0.0f }, 
        int numSegments = 16
    );
};