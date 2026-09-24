#pragma once

#include "assets/asset.hpp"

#include <string>
#include <vector>
#include <DirectXMath.h>

enum class AnimationPath {
    TRANSLATION,
    ROTATION,
    SCALE
};

enum class AnimationInterpolation {
    STEP,
    LINEAR
};

// Keyframes for one property of one joint
struct AnimationChannel {
    std::string jointName;

    AnimationPath pathMode;
    AnimationInterpolation interpolationMode;

    std::vector<float> times;
    std::vector<DirectX::XMFLOAT4> values;
};

struct AnimationClip {
    std::string name;
    float duration = 0.0f;
    std::vector<AnimationChannel> channels;
};

class AnimationLibrary : public Asset {
    std::vector<AnimationClip> clips;

public:
    AnimationLibrary() = default;

    bool Create(std::vector<AnimationClip> &&clips);

    void DrawImGui() override;

    int32_t FindClip(const std::string &name) const;

    size_t GetClipCount() const { return this->clips.size(); }
    const std::vector<AnimationClip> &GetClips() const { return this->clips; }
    const AnimationClip &GetClip(uint32_t index) const;
};