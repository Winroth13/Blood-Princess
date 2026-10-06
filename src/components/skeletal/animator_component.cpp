#include "animator_component.hpp"
#include "assets/types/model.hpp"
#include "assets/types/animation_library.hpp"
#include "components/core/model_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"

#include <algorithm>
#include <cmath>

using namespace DirectX;

static XMVECTOR SampleChannel(const AnimationChannel &channel, float time) {
    const std::vector<float> &times = channel.times;

    if (times.size() == 1 || time <= times.front())
        return XMLoadFloat4(&channel.values.front());

    if (time >= times.back())
        return XMLoadFloat4(&channel.values.back());

    const size_t next = std::upper_bound(times.begin(), times.end(), time) - times.begin();
    const size_t prev = next - 1;

    const XMVECTOR prevV = XMLoadFloat4(&channel.values[prev]);

    if (channel.interpolation == AnimationInterpolation::STEP)
        return prevV;

    const XMVECTOR nextV = XMLoadFloat4(&channel.values[next]);

    const float span = times[next] - times[prev];
    const float t = span > 0.0f ? (time - times[prev]) / span : 0.0f;

    if (channel.path == AnimationPath::ROTATION)
        return XMQuaternionSlerp(prevV, nextV, t);

    return XMVectorLerp(prevV, nextV, t);
}

void AnimatorComponent::LateUpdate(const AppContext &context, float deltaTime) {
    this->RefreshSkeleton();
    if (this->skeleton == nullptr)
        return;

    if (this->currentClip && !this->isPaused)
        this->SetTime(this->time + this->speed * deltaTime);

    this->ResetToBindPose();

    if (this->currentClip)
        this->SampleClip(*this->currentClip, this->time);

    this->UpdateJointModelMatrices();
    this->UpdateSkinMatrices();
}

void AnimatorComponent::RefreshSkeleton() {
    ModelComponent *modelComponent = this->GetOwner()->GetComponent<ModelComponent>();
    std::shared_ptr<Model> model = modelComponent ? modelComponent->GetModel() : nullptr;
    std::shared_ptr<Skeleton> skeleton = model ? model->GetSkeleton() : nullptr;

    if (skeleton != this->skeleton) {
        this->skeleton = skeleton;

        for (auto &[name, binding] : this->clips)
            this->BindClip(binding);

        const size_t jointCount = skeleton ? skeleton->GetJointCount() : 0;
        this->localPose.resize(jointCount);
        this->jointModelMatrices.resize(jointCount);
        this->skinMatrices.resize(skeleton ? skeleton->GetSkinJointCount() : 0);
    }

    if (!this->skeleton && !this->hasWarnedMissingSkeleton) {
        LogWarn("AnimatorComponent on entity '%s' needs a ModelComponent with a skinned model", this->GetOwner()->GetName().c_str());
        this->hasWarnedMissingSkeleton = true;
    }
}

void AnimatorComponent::BindClip(ClipBinding &binding) {
    binding.channelJoints.clear();

    if (!this->skeleton)
        return;

    const AnimationClip &clip = binding.animations->GetClip(binding.clipIndex);
    binding.channelJoints.reserve(clip.channels.size());

    for (const AnimationChannel &channel : clip.channels)
        binding.channelJoints.push_back(this->skeleton->FindJoint(channel.jointName));
}

void AnimatorComponent::ResetToBindPose() {
    const std::vector<Joint> &joints = this->skeleton->GetJoints();

    for (size_t i = 0; i < joints.size(); ++i) {
        this->localPose[i].translation = joints[i].bindTransform.translation;
        this->localPose[i].rotation = joints[i].bindTransform.rotation;
        this->localPose[i].scale = joints[i].bindTransform.scale;
    }
}

void AnimatorComponent::SampleClip(const ClipBinding &binding, float time) {
    const AnimationClip &clip = binding.animations->GetClip(binding.clipIndex);

    if (binding.channelJoints.size() != clip.channels.size())
        return;

    for (size_t i = 0; i < clip.channels.size(); ++i) {
        const int32_t jointIndex = binding.channelJoints[i];
        if (jointIndex < 0)
            continue;

        const AnimationChannel &channel = clip.channels[i];
        const XMVECTOR value = SampleChannel(channel, time);

        JointTransform &transform = this->localPose[jointIndex];
        switch (channel.path) {
            case AnimationPath::TRANSLATION: {
                XMStoreFloat3(&transform.translation, value);
            } break;
            case AnimationPath::ROTATION: {
                XMStoreFloat4(&transform.rotation, value);
            } break;
            case AnimationPath::SCALE: {
                XMStoreFloat3(&transform.scale, value);
            } break;
        }
    }
}

void AnimatorComponent::UpdateJointModelMatrices() {
    const std::vector<Joint> &joints = this->skeleton->GetJoints();

    for (size_t i = 0; i < joints.size(); ++i) {
        const JointTransform &transform = this->localPose[i];

        XMMATRIX matrix =
            XMMatrixScalingFromVector(XMLoadFloat3(&transform.scale)) *
            XMMatrixRotationQuaternion(XMLoadFloat4(&transform.rotation)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&transform.translation)
        );

        const int32_t parentIndex = joints[i].parentIndex;
        if (parentIndex != -1)
            matrix = matrix * XMLoadFloat4x4(&this->jointModelMatrices[parentIndex]);

        XMStoreFloat4x4(&this->jointModelMatrices[i], matrix);
    }
}

void AnimatorComponent::UpdateSkinMatrices() {
    const std::vector<int32_t> &skinJoints = this->skeleton->GetSkinJoints();
    const std::vector<XMFLOAT4X4> &inverseBindMatrices = this->skeleton->GetInverseBindMatrices();

    for (size_t i = 0; i < skinJoints.size(); ++i) {
        const XMMATRIX matrix = XMLoadFloat4x4(&inverseBindMatrices[i]) * XMLoadFloat4x4(&this->jointModelMatrices[skinJoints[i]]);
        XMStoreFloat4x4(&this->skinMatrices[i], matrix);
    }
}

bool AnimatorComponent::AddAnimation(const std::string &name, std::shared_ptr<AnimationLibrary> animations, uint32_t clipIndex) {
    if (!animations || clipIndex >= animations->GetClipCount())
        return false;

    ClipBinding &binding = this->clips[name];
    binding.animations = animations;
    binding.clipIndex = clipIndex;

    this->BindClip(binding);

    return true;
}

void AnimatorComponent::AddAnimations(std::shared_ptr<AnimationLibrary> animations) {
    if (!animations)
        return;

    for (uint32_t i = 0; i < animations->GetClipCount(); ++i) {
        const std::string &name = animations->GetClip(i).name;

        if (this->HasAnimation(name))
            LogWarn("Replaced animation '%s' on entity '%s'", name.c_str(), this->GetOwner()->GetName().c_str());

        this->AddAnimation(name, animations, i);
    }
}

bool AnimatorComponent::Play(const std::string &name, bool isLooping) {
    auto iter = this->clips.find(name);
    if (iter == this->clips.end()) {
        LogWarn("Animator on entity '%s' has no animation '%s'", this->GetOwner()->GetName().c_str(), name.c_str());
        return false;
    }

    this->currentClip = &iter->second;
    this->currentClipName = name;
    this->isLooping = isLooping;
    this->time = 0.0f;

    return true;
}

void AnimatorComponent::Stop() {
    this->currentClip = nullptr;
    this->currentClipName.clear();
    this->time = 0.0f;
}

bool AnimatorComponent::IsFinished() const {
    return this->currentClip && !this->isLooping && this->time >= this->GetDuration();
}

void AnimatorComponent::SetTime(float time) {
    const float duration = this->GetDuration();

    if (duration <= 0.0f) {
        this->time = 0.0f;
        return;
    }

    if (this->isLooping) {
        time = fmodf(time, duration);
        if (time < 0.0f)
            time += duration;
    }
    else {
        time = std::clamp(time, 0.0f, duration);
    }

    this->time = time;
}

float AnimatorComponent::GetDuration() const {
    if (!this->currentClip)
        return 0.0f;

    return this->currentClip->animations->GetClip(this->currentClip->clipIndex).duration;
}

int32_t AnimatorComponent::FindJoint(const std::string &name) const {
    return this->skeleton ? this->skeleton->FindJoint(name) : -1;
}

DirectX::XMMATRIX AnimatorComponent::GetJointModelMatrix(int32_t jointIndex) const {
    if (jointIndex < 0 || static_cast<size_t>(jointIndex) >= this->jointModelMatrices.size())
        return XMMatrixIdentity();

    return XMLoadFloat4x4(&this->jointModelMatrices[jointIndex]);
}