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

    static std::map<std::string, bool> boolSettings;
    static std::map<std::string, int> intSettings;
    static std::map<std::string, float> floatSettings;
    static std::map<std::string, DirectX::XMFLOAT2> float2Settings;
    static std::map<std::string, DirectX::XMFLOAT3> float3Settings;
    static std::map<std::string, DirectX::XMFLOAT4> float4Settings;

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

    static bool GetSetting(const std::string &name, bool defaultValue);
    static int GetSetting(const std::string &name, int defaultValue);
    static float GetSetting(const std::string &name, float defaultValue);
    static DirectX::XMFLOAT2 GetSetting(const std::string &name, const DirectX::XMFLOAT2 &defaultValue);
    static DirectX::XMFLOAT3 GetSetting(const std::string &name, const DirectX::XMFLOAT3 &defaultValue);
    static DirectX::XMFLOAT4 GetSetting(const std::string &name, const DirectX::XMFLOAT4 &defaultValue);
    
    static void SetSetting(const std::string &name, bool value);
    static void SetSetting(const std::string &name, int value);
    static void SetSetting(const std::string &name, float value);
    static void SetSetting(const std::string &name, DirectX::XMFLOAT2 value);
    static void SetSetting(const std::string &name, DirectX::XMFLOAT3 value);
    static void SetSetting(const std::string &name, DirectX::XMFLOAT4 value);

    static std::map<std::string, bool> &GetBoolSettings();
    static std::map<std::string, int> &GetIntSettings();
    static std::map<std::string, float> &GetFloatSettings();
    static std::map<std::string, DirectX::XMFLOAT2> &GetFloat2Settings();
    static std::map<std::string, DirectX::XMFLOAT3> &GetFloat3Settings();
    static std::map<std::string, DirectX::XMFLOAT4> &GetFloat4Settings();
};