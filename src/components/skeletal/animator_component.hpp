#pragma once

#include "scene/component.hpp"
#include "assets/types/sub_types/skeleton.hpp"

#include <unordered_map>

class AnimationLibrary;

class AnimatorComponent : public Component {
    RegisterComponent(AnimatorComponent);

    ExportPropertyRanged(float, speed, "x", 0.0f, 0.0f, 0.01f, 1.0f);
    ExportProperty(bool, isPaused, false); // Pose still needs to be evaluated so that e.g. SetTime() works
    ExportProperty(bool, isSkeletonVisible, false); // Debug

    struct ClipBinding {
        std::shared_ptr<AnimationLibrary> animations = nullptr;
        uint32_t clipIndex = 0;
        std::vector<int32_t> channelJoints;
    };

    struct ActiveClip {
        ClipBinding *binding = nullptr;
        std::string name;
        float time = 0.0f;
        bool isLooping = true;

        float weight = 1.0f;
        float fadeStartWeight = 0.0f;
    };

    static constexpr size_t MAX_ACTIVE_CLIP_COUNT = 4;

    std::shared_ptr<Skeleton> skeleton = nullptr;
    bool hasWarnedMissingSkeleton = false;

    std::unordered_map<std::string, ClipBinding> clips;

    std::vector<ActiveClip> activeClips;
    float fadeTime = 0.0f;
    float fadeDuration = 0.0f;

    std::vector<JointTransform> localPose;
    std::vector<JointTransform> blendPose;
    std::vector<DirectX::XMFLOAT4X4> jointModelMatrices;
    std::vector<DirectX::XMFLOAT4X4> skinMatrices;

public:
    AnimatorComponent(Entity *owner, bool isActive) : Component(owner, isActive) { 
        UsesLateUpdate(); 
        this->activeClips.reserve(MAX_ACTIVE_CLIP_COUNT + 1);
    }

private:
    void LateUpdate(const AppContext &context, float deltaTime) override;

    void RefreshSkeleton();
    void BindClip(ClipBinding &binding);

    ClipBinding *FindBinding(const std::string &name);
    float GetClipDuration(const ActiveClip &clip) const;
    float WrapTime(const ActiveClip &clip, float time) const;

    void BeginFade(ClipBinding *binding, const std::string &name, bool isLooping, float fadeDuration);
    void UpdateFadeWeights();

    void ResetToBindPose(std::vector<JointTransform> &pose) const;
    void SampleClip(const ClipBinding &binding, float time, std::vector<JointTransform> &pose);
    void UpdateLocalPose();
    void UpdateJointModelMatrices();
    void UpdateSkinMatrices();

public:
    bool AddAnimation(const std::string &name, std::shared_ptr<AnimationLibrary> animations, uint32_t clipIndex = 0);
    void AddAnimations(std::shared_ptr<AnimationLibrary> animations);
    bool HasAnimation(const std::string &name) const { return this->clips.find(name) != this->clips.end(); }

    // Switches animation instantly, ending any current cross fade. 
    // If it's the current animation, nothing happens if it's looping while one-shots reset.
    bool Play(const std::string &name, bool isLooping = true);
    // Same as Play() but blends the current animation into the new one over the duration.
    bool CrossFade(const std::string &name, float fadeDuration, bool isLooping = true);
    // Back to bind pose
    void Stop();

    bool IsPlaying() const { return !this->activeClips.empty(); }
    bool IsPlaying(const std::string &name) const { return this->IsPlaying() && this->activeClips.back().name == name; }
    bool IsFading() const { return this->activeClips.size() > 1; }
    bool IsFinished() const;
    const std::string &GetCurrentAnimationName() const;

    float GetTime() const { return this->IsPlaying() ? this->activeClips.back().time : 0.0f; }
    void SetTime(float time);
    float GetDuration() const;
    float GetNormalizedTime() const;

    float GetSpeed() const { return this->speed; }
    void SetSpeed(float speed) { this->speed = speed; }

    bool IsPaused() const { return this->isPaused; }
    void SetPaused(bool isPaused) { this->isPaused = isPaused; }

    bool IsSkeletonVisible() const { return this->isSkeletonVisible; }
    void SetSkeletonVisible(bool isSkeletonVisible) { this->isSkeletonVisible = isSkeletonVisible; }

    std::shared_ptr<Skeleton> GetSkeleton() const { return this->skeleton; }
    int32_t FindJoint(const std::string &name) const;

    const std::vector<DirectX::XMFLOAT4X4> &GetJointModelMatrices() const { return this->jointModelMatrices; }
    DirectX::XMMATRIX GetJointModelMatrix(int32_t jointIndex) const;
    const std::vector<DirectX::XMFLOAT4X4> &GetSkinMatrices() const { return this->skinMatrices; }
};