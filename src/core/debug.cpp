#include "debug.hpp"

#include <format>

std::unordered_map<std::string, std::string> Debug::statistics;

void Debug::NewFrame() {
    statistics.clear();
}

void Debug::SetStatistic(const std::string &name, const std::string &value) {
    statistics[name] = value;
}

void Debug::SetStatistic(const std::string &name, int value) {
    statistics[name] = std::to_string(value);
}

void Debug::SetStatistic(const std::string &name, bool value) {
    statistics[name] = value ? "True" : "False";
}

void Debug::SetStatistic(const std::string &name, float value) {
    statistics[name] = std::format("{:.3f}", value);
}

void Debug::SetStatistic(const std::string &name, const DirectX::XMFLOAT2 &value) {
    statistics[name] = std::format("[{:.3f}, {:.3f}]", value.x, value.y);
}

void Debug::SetStatistic(const std::string &name, const DirectX::XMFLOAT3 &value) {
    statistics[name] = std::format("[{:.3f}, {:.3f}, {:.3f}]", value.x, value.y, value.z);
}

void Debug::SetStatistic(const std::string &name, const DirectX::XMFLOAT4 &value) {
    statistics[name] = std::format("[{:.3f}, {:.3f}, {:.3f}, {:.3f}]", value.x, value.y, value.z, value.w);
}

const std::unordered_map<std::string, std::string> &Debug::GetStatistic() {
    return statistics;
}