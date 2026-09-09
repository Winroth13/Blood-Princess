#pragma once

#include "rendering/render_queue.hpp"

#include <DirectXCollision.h>

enum class ViewType 
{
    PRIMARY,
    SHADOW_MAP_POINT
};

struct RenderView 
{
    ViewType type = ViewType::PRIMARY;
    int index = 0;
    
    DirectX::XMFLOAT4X4 viewMatrix{};
    DirectX::XMFLOAT4X4 projectionMatrix{};

    DirectX::XMFLOAT3 cameraPosition{};
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    DirectX::BoundingFrustum frustum;
    bool skipFrustumCulling = false; // TODO: Orthographic views don't work with BoundingFrustum; could use OBB instead

    // float shadowDistance = 80.0f;

    RenderQueue queue;
};