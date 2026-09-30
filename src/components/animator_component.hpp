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

    std::shared_ptr<Skeleton> skeleton = nullptr;
    bool hasWarnedMissingSkeleton = false;

    std::unordered_map<std::string, ClipBinding> clips;

    ClipBinding *currentClip = nullptr;
    std::string currentClipName;
    float time = 0.0f;
    bool isLooping = true;

    std::vector<JointTransform> localPose;
    std::vector<DirectX::XMFLOAT4X4> jointModelMatrices;

public:
    AnimatorComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoRender(); }

private:
    void LateUpdate(const AppContext &context, float deltaTime) override;

    void RefreshSkeleton();
    void BindClip(ClipBinding &binding);

    void ResetToBindPose();
    void SampleClip(const ClipBinding &binding, float time);
    void UpdateJointModelMatrices();

public:
    bool AddAnimation(const std::string &name, std::shared_ptr<AnimationLibrary> animations, uint32_t clipIndex = 0);
    void AddAnimations(std::shared_ptr<AnimationLibrary> animations);
    bool HasAnimation(const std::string &name) const { return this->clips.find(name) != this->clips.end(); }

    bool Play(const std::string &name, bool isLooping = true);
    void Stop();

    bool IsPlaying() const { return this->currentClip != nullptr; }
    bool IsFinished() const;
    const std::string &GetCurrentAnimationName() const { return this->currentClipName; }

    float GetTime() const { return this->time; }
    void SetTime(float time);
    float GetDuration() const;

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
};