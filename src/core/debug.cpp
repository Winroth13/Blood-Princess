#include "debug.hpp"

#include <format>

std::vector<std::pair<std::string, std::string>> Debug::statistics;
std::pair<std::string, std::chrono::steady_clock::time_point> Debug::statisticPair;
std::stack<std::pair<std::string, std::chrono::steady_clock::time_point>> Debug::statisticStack;

std::map<std::string, bool> Debug::settings;

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

std::string Debug::GetStackIndentedname(const std::string &name) {
    std::string indentedName = name;
    if (statisticStack.size() > 0)
        indentedName = "  > " + indentedName;
    for (size_t i = statisticStack.size(); i > 1; i--)
        indentedName = "\t" + indentedName;
    return indentedName;
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

void Debug::BeginTimedStatistic(const std::string &name) {
    statisticPair.first = GetStackIndentedname(name);
    statisticPair.second = std::chrono::high_resolution_clock::now();
    statisticStack.push(statisticPair);

    Debug::SetStatistic(statisticPair.first + " (ms)", "-");
}

void Debug::EndTimedStatistic() {
    statisticPair = statisticStack.top();

    std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - statisticPair.second;
    std::chrono::microseconds msTime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);

    Debug::SetStatistic(statisticPair.first + " (ms)", static_cast<float>(msTime.count()) / 1000);

    statisticStack.pop();
}

bool Debug::GetSetting(const std::string &name, bool defaultValue) {
    auto iter = settings.find(name);
    if (iter != settings.end())
        return iter->second;

    return settings[name] = defaultValue;
}

void Debug::SetSetting(const std::string &name, bool value) {
    settings[name] = value; 
}

std::map<std::string, bool> &Debug::GetSettings() {
    return settings;
}