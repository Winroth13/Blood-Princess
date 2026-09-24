#include "animation_library.hpp"
#include "core/logging.hpp"
#include "imgui/imgui.h"

bool AnimationLibrary::Create(std::vector<AnimationClip> &&clips) {
    for (const AnimationClip &clip : clips) {
        for (const AnimationChannel &channel : clip.channels) {
            if (channel.times.empty() || channel.times.size() != channel.values.size()) {
                LogWarn("Animation clip '%s' has a channel for joint '%s' with '%zu' times and '%zu' values (should be equal)",
                    clip.name.c_str(), channel.jointName.c_str(), channel.times.size(), channel.values.size()
                );
                return false;
            }
        }
    }

    this->clips = std::move(clips);

    return true;
}

void AnimationLibrary::DrawImGui() {
    ImGui::Text("Hello from an Animation Library. I have '%zu' clips :)", this->clips.size());
}

int32_t AnimationLibrary::FindClip(const std::string &name) const {
    for (size_t i = 0; i < this->clips.size(); ++i)
        if (this->clips[i].name == name)
            return static_cast<int32_t>(i);

    return -1;
}

const AnimationClip &AnimationLibrary::GetClip(uint32_t index) const {
    if (index < 0 || index >= this->clips.size()) {
        LogWarn("Animation clip index '%u' out of range [0, %zu)", index, this->clips.size());
        return {};
    }

    return this->clips[index];
}
