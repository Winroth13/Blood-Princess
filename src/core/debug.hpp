#pragma once

#include <vector>
#include <string>
#include <DirectXMath.h>
#include <stack>
#include <chrono>
#include <map>

class Debug {
    friend class Application;

    static std::vector<std::pair<std::string, std::string>> statistics;
    static std::pair<std::string, std::chrono::steady_clock::time_point> statisticPair;
    static std::stack<std::pair<std::string, std::chrono::steady_clock::time_point>> statisticStack;

    static std::map<std::string, bool> settings;

    static void NewFrame();

    static void PushStatistic(const std::string &name, const std::string &value);

    static std::string GetStackIndentedname(const std::string &name);

public:
    static void SetStatistic(const std::string &name, const std::string &value);
    static void SetStatistic(const std::string &name, int value);
    static void SetStatistic(const std::string &name, bool value);
    static void SetStatistic(const std::string &name, float value);
    static void SetStatistic(const std::string &name, const DirectX::XMFLOAT2 &value);
    static void SetStatistic(const std::string &name, const DirectX::XMFLOAT3 &value);
    static void SetStatistic(const std::string &name, const DirectX::XMFLOAT4 &value);

    static const std::vector<std::pair<std::string, std::string>> &GetStatistic();

    static void BeginTimedStatistic(const std::string &name);
    static void EndTimedStatistic();

    static bool GetSetting(const std::string &name, bool defaultValue = false);
    static void SetSetting(const std::string &name, bool value);

    static std::map<std::string, bool> &GetSettings();
};