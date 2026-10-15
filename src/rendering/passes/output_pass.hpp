#pragma once

#include "rendering/shared_resources.hpp"
#include "rendering/resources/backbuffer.hpp"
#include "rendering/passes/deferred_pass.hpp"
#include "core/debug.hpp"

#include "d3d11.h"

#include <memory>
#include <cstdint>

class AssetManager;
class ComputeShader;

struct DebugViewData {
    int debugView;
    float pad0[3];
};

struct PostProcessingData {
    XMFLOAT4 speedLineColour;
    float speedLineCount;
    float speedLineDensity;
    float speedLineFaloff;
    float speedLineMaskSize;
    float speedLineMaskEdge;
    float speedLineAnimationSpeed;
    float damageFlashIntensity;
    float damageFlashEdgePower;
    XMFLOAT4 damageFlashColour;
    float damageFlashInnerRatio;
    float damageFlashFeather;
    float pad0[2];
};



enum class DebugView : int {
    DEFAULT,
    ALBEDO,
    METALLIC,
    NORMAL,
    ROUGHNESS,
    DEPTH,
    QUAD,

    COUNT
};

static constexpr int operator+(DebugView val) noexcept {
    return static_cast<int>(val);
}

class OutputPass {
    std::shared_ptr<Texture2D> noiseTexture;
    std::shared_ptr<ComputeShader> computeShader;

    uint32_t width = 0;
    uint32_t height = 0;

    DebugView debugView = DebugView::DEFAULT;
    ID3D11Buffer *debugCB = nullptr;
    ID3D11Buffer *postProcessingCB = nullptr;

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

    DebugView GetDebugView() const { return this->debugView; }
    void SetDebugView(DebugView debugView) { this->debugView = debugView; }
};