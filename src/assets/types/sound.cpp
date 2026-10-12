#include "sound.hpp"
#include "core/logging.hpp"

bool Sound::Create(
    XMFLOAT2 volumeRange,
    XMFLOAT2 pitchRange,
    const std::vector<Sound::Layer> &soundLayers)
{
    this->volumeRange = volumeRange;
    this->pitchRange = pitchRange;

    if (soundLayers.size() == 0)
    {
        LogError("Sound Layers size was 0");
        return false;
    }

    this->soundLayers = soundLayers;

    return true;
}
