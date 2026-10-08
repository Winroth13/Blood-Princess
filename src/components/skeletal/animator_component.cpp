#include "animator_component.hpp"
#include "assets/types/model.hpp"
#include "assets/types/animation_library.hpp"
#include "components/core/model_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"

#include <algorithm>
#include <cmath>

using namespace DirectX;

namespace {
    XMVECTOR SampleChannel(const AnimationChannel &channel, float time) {
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

    float Smoothstep(float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
}

void AnimatorComponent::LateUpdate(const AppContext &context, float deltaTime) {
    this->RefreshSkeleton();
    if (this->skeleton == nullptr)
        return;

    if (!this->isPaused) {
        for (ActiveClip &clip : this->activeClips)
            clip.time = this->WrapTime(clip, clip.time + this->speed * deltaTime);

        if (this->IsFading()) {
            this->fadeTime += deltaTime;
            this->UpdateFadeWeights();
        }
    }
    
    this->UpdateLocalPose();
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
        this->blendPose.resize(jointCount);
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

AnimatorComponent::ClipBinding *AnimatorComponent::FindBinding(const std::string &name) {
    auto iter = this->clips.find(name);
    if (iter == this->clips.end()) {
        LogWarn("Animator on entity '%s' has no animation '%s'", this->GetOwner()->GetName().c_str(), name.c_str());
        return nullptr;
    }

    return &iter->second;
}

float AnimatorComponent::GetClipDuration(const ActiveClip &clip) const {
    return clip.binding->animations->GetClip(clip.binding->clipIndex).duration;
}

float AnimatorComponent::WrapTime(const ActiveClip &clip, float time) const {
    const float duration = this->GetClipDuration(clip);

    if (duration <= 0.0f)
        return 0.0f;

    if (clip.isLooping) {
        time = fmodf(time, duration);
        if (time < 0.0f)
            time += duration;

        return time;
    }

    return std::clamp(time, 0.0f, duration);
}

void AnimatorComponent::BeginFade(ClipBinding *binding, const std::string &name, bool isLooping, float fadeDuration) {
    ActiveClip clip{};
    clip.binding = binding;
    clip.name = name;
    clip.isLooping = isLooping;
    clip.weight = 0.0f;

    if (isLooping) {
        auto revived = this->activeClips.end();
        for (auto iter = this->activeClips.begin(); iter != this->activeClips.end(); ++iter)
            if (iter->isLooping && iter->name == clip.name && (revived == this->activeClips.end() || iter->weight > revived->weight))
                revived = iter;

        if (revived != this->activeClips.end()) {
            clip = *revived;
            this->activeClips.erase(revived);
        }
    }

    for (ActiveClip &active : this->activeClips)
        active.fadeStartWeight = active.weight;
    clip.fadeStartWeight = clip.weight;

    std::erase_if(this->activeClips, [](const ActiveClip &active) { return active.weight <= 0.0f; });

    this->activeClips.push_back(clip);

    while (this->activeClips.size() > MAX_ACTIVE_CLIP_COUNT) {
        auto weakest = std::min_element(
            this->activeClips.begin(), 
            this->activeClips.end() - 1, 
            [](const ActiveClip &a, const ActiveClip &b) { return a.weight < b.weight; }
        );
        this->activeClips.erase(weakest);
    }

    float totalWeight = 0.0f;
    for (const ActiveClip &active : this->activeClips)
        totalWeight += active.fadeStartWeight;

    if (totalWeight > 0.0f) {
        for (ActiveClip &active : this->activeClips) {
            active.fadeStartWeight /= totalWeight;
            active.weight = active.fadeStartWeight;
        }
    }

    this->fadeTime = 0.0f;
    this->fadeDuration = fadeDuration;
}

void AnimatorComponent::UpdateFadeWeights() {
    const float alpha = Smoothstep(this->fadeDuration > 0.0f ? this->fadeTime / this->fadeDuration : 1.0f);

    if (alpha >= 1.0f) {
        this->activeClips.erase(this->activeClips.begin(), this->activeClips.end() - 1);
        this->activeClips.back().weight = 1.0f;
        return;
    }

    for (ActiveClip &clip : this->activeClips)
        clip.weight = clip.fadeStartWeight * (1.0f - alpha);

    ActiveClip &current = this->activeClips.back();
    current.weight = current.fadeStartWeight + (1.0f - current.fadeStartWeight) * alpha;
}

void AnimatorComponent::ResetToBindPose(std::vector<JointTransform> &pose) const {
    const std::vector<Joint> &joints = this->skeleton->GetJoints();
    for (size_t i = 0; i < joints.size(); ++i)
        pose[i] = joints[i].bindTransform;
}

void AnimatorComponent::SampleClip(const ClipBinding &binding, float time, std::vector<JointTransform> &pose) {
    const AnimationClip &clip = binding.animations->GetClip(binding.clipIndex);

    if (binding.channelJoints.size() != clip.channels.size())
        return;

    for (size_t i = 0; i < clip.channels.size(); ++i) {
        const int32_t jointIndex = binding.channelJoints[i];
        if (jointIndex < 0)
            continue;

        const AnimationChannel &channel = clip.channels[i];
        const XMVECTOR value = SampleChannel(channel, time);

        JointTransform &transform = pose[jointIndex];
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

void AnimatorComponent::UpdateLocalPose() {
    this->ResetToBindPose(this->localPose);

    if (this->activeClips.empty())
        return;

    const ActiveClip &first = this->activeClips.front();
    this->SampleClip(*first.binding, first.time, this->localPose);

    float accumulatedWeight = first.weight;

    for (size_t i = 1; i < this->activeClips.size(); ++i) {
        const ActiveClip &clip = this->activeClips[i];
        if (clip.weight <= 0.0f)
            continue;

        accumulatedWeight += clip.weight;
        const float t = clip.weight / accumulatedWeight;

        this->ResetToBindPose(this->blendPose);
        this->SampleClip(*clip.binding, clip.time, this->blendPose);

        for (size_t j = 0; j < this->localPose.size(); ++j) {
            JointTransform &result = this->localPose[j];
            const JointTransform &other = this->blendPose[j];

            XMStoreFloat3(&result.translation, XMVectorLerp(XMLoadFloat3(&result.translation), XMLoadFloat3(&other.translation), t));
            XMStoreFloat3(&result.scale, XMVectorLerp(XMLoadFloat3(&result.scale), XMLoadFloat3(&other.scale), t));

            const XMVECTOR rotation = XMLoadFloat4(&result.rotation);
            XMVECTOR otherRotation = XMLoadFloat4(&other.rotation);
            if (XMVectorGetX(XMVector4Dot(rotation, otherRotation)) < 0.0f)
                otherRotation = XMVectorNegate(otherRotation);

            XMStoreFloat4(&result.rotation, XMQuaternionNormalize(XMVectorLerp(rotation, otherRotation, t)));
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
    ClipBinding *binding = this->FindBinding(name);
    if (!binding)
        return false;

    ActiveClip clip{};
    clip.binding = binding;
    clip.name = name;
    clip.isLooping = isLooping;
    clip.weight = 1.0f;
    clip.fadeStartWeight = 1.0f;

    if (isLooping && this->IsPlaying(name) && this->activeClips.back().isLooping)
        clip.time = this->activeClips.back().time;

    this->activeClips.clear();
    this->activeClips.push_back(clip);

    this->fadeTime = 0.0f;
    this->fadeDuration = 0.0f;

    return true;
}

bool AnimatorComponent::CrossFade(const std::string &name, float fadeDuration, bool isLooping) {
    if (fadeDuration <= 0.0f || !this->IsPlaying())
        return this->Play(name, isLooping);

    ClipBinding *binding = this->FindBinding(name);
    if (!binding)
        return false;

    if (isLooping && this->IsPlaying(name) && this->activeClips.back().isLooping)
        return true;

    this->BeginFade(binding, name, isLooping, fadeDuration);

    return true;
}

void AnimatorComponent::Stop() {
    this->activeClips.clear();
    this->fadeTime = 0.0f;
    this->fadeDuration = 0.0f;
}

bool AnimatorComponent::IsFinished() const {
    if (!this->IsPlaying())
        return false;

    const ActiveClip &clip = this->activeClips.back();
    return !clip.isLooping && clip.time >= this->GetClipDuration(clip);
}

const std::string &AnimatorComponent::GetCurrentAnimationName() const {
    static const std::string NONE = "";
    return this->IsPlaying() ? this->activeClips.back().name : NONE;
}

void AnimatorComponent::SetTime(float time) {
    if (!this->IsPlaying())
        return;

    ActiveClip &clip = this->activeClips.back();
    clip.time = this->WrapTime(clip, time);
}

float AnimatorComponent::GetDuration() const {
    return this->IsPlaying() ? this->GetClipDuration(this->activeClips.back()) : 0.0f;
}

float AnimatorComponent::GetNormalizedTime() const {
    const float duration = this->GetDuration();
    return duration > 0.0f ? this->GetTime() / duration : 0.0f;
}

int32_t AnimatorComponent::FindJoint(const std::string &name) const {
    return this->skeleton ? this->skeleton->FindJoint(name) : -1;
}

DirectX::XMMATRIX AnimatorComponent::GetJointModelMatrix(int32_t jointIndex) const {
    if (jointIndex < 0 || static_cast<size_t>(jointIndex) >= this->jointModelMatrices.size())
        return XMMatrixIdentity();

    return XMLoadFloat4x4(&this->jointModelMatrices[jointIndex]);
}