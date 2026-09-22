#pragma once

#include "rendering/shared_resources.hpp"
#include "rendering/resources/backbuffer.hpp"
#include "rendering/passes/deferred_pass.hpp"

#include "d3d11.h"

#include <memory>
#include <cstdint>

class AssetManager;
class ComputeShader;

class OutputPass {
    std::shared_ptr<ComputeShader> computeShader;

    uint32_t width = 0;
    uint32_t height = 0;

public:
    OutputPass() = default;
    ~OutputPass() = default;

    OutputPass(const OutputPass &other) = delete;
    OutputPass &operator=(const OutputPass &other) = delete;

    bool Initialize(
        ID3D11Device *device,
        AssetManager *assetManager,
        uint32_t width,
        uint32_t height
    );

    void Shutdown();

    bool Resize(
        ID3D11Device *device,
        uint32_t width,
        uint32_t height
    );

    void Render(
        ID3D11Device *device,
        ID3D11DeviceContext *deviceContext,
        SharedResources &sharedResources,
        DeferredPass &deferredPass,
        BackBuffer &backBuffer,
        std::vector<RenderView> &views
    );
};