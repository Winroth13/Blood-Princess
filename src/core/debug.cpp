#include "debug.hpp"

#include <format>

std::vector<std::pair<std::string, std::string>> Debug::statistics;

void Debug::NewFrame() {
    statistics.clear();
}

void Debug::PushStatistic(const std::string &name, const std::string &value) {
    for (std::pair<std::string, std::string> &pair : statistics)
    {
        if (pair.first == name)
        {
            pair.second = value;
            return;
        }
    }

    statistics.push_back(std::make_pair(name, value));
}

void Debug::SetStatistic(const std::string &name, const std::string &value) {
    PushStatistic(name, value);
}

void Debug::SetStatistic(const std::string &name, int value) {
    PushStatistic(name, std::to_string(value));
}

void Debug::SetStatistic(const std::string &name, bool value) {
    PushStatistic(name, value ? "True" : "False");
}

void Debug::SetStatistic(const std::string &name, float value) {
    PushStatistic(name, std::format("{:.3f}", value));
}

void Debug::SetStatistic(const std::string &name, const DirectX::XMFLOAT2 &value) {
    PushStatistic(name, std::format("[{:.3f}, {:.3f}]", value.x, value.y));
}

void Debug::SetStatistic(const std::string &name, const DirectX::XMFLOAT3 &value) {
    PushStatistic(name, std::format("[{:.3f}, {:.3f}, {:.3f}]", value.x, value.y, value.z));
}

void Debug::SetStatistic(const std::string &name, const DirectX::XMFLOAT4 &value) {
    PushStatistic(name, std::format("[{:.3f}, {:.3f}, {:.3f}, {:.3f}]", value.x, value.y, value.z, value.w));
}

const std::vector<std::pair<std::string, std::string>> &Debug::GetStatistic() {
    return statistics;
}