#pragma once

#include <vector>
#include <string>
#include <DirectXMath.h>

class Debug {
    friend class Application;

    static std::vector<std::pair<std::string, std::string>> statistics;

    static void NewFrame();

    static void PushStatistic(const std::string &name, const std::string &value);

public:
    static void SetStatistic(const std::string &name, const std::string &value);
    static void SetStatistic(const std::string &name, int value);
    static void SetStatistic(const std::string &name, bool value);
    static void SetStatistic(const std::string &name, float value);
    static void SetStatistic(const std::string &name, const DirectX::XMFLOAT2 &value);
    static void SetStatistic(const std::string &name, const DirectX::XMFLOAT3 &value);
    static void SetStatistic(const std::string &name, const DirectX::XMFLOAT4 &value);

    static const std::vector<std::pair<std::string, std::string>> &GetStatistic();
};