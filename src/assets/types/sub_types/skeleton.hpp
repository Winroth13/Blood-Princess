#pragma once

#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <string>

using namespace DirectX;

struct BindTransform {
    XMFLOAT3 translation;
    XMFLOAT4 rotation; 
    XMFLOAT3 scale;
};

struct Joint {
    std::string name;
    int32_t parentIndex = -1;
    BindTransform bindTransform;
};

class Skeleton {
    std::vector<Joint> joints;
    std::unordered_map<std::string, int32_t> jointIndeces;

    std::vector<int32_t> skinJointToJoint;
    std::vector<int32_t> jointToSkinJoint;

    std::vector<XMFLOAT4X4> inverseBindMatrices;

public:
    Skeleton() = default;

    bool Create(
        const std::vector<Joint> &joints, 
        const std::vector<int32_t> &skinJointToJoint, 
        const std::vector<XMFLOAT4X4> &inverseBindMatrices
    );

    int32_t FindJoint(const std::string &name) const;

    size_t GetJointCount() const { return this->joints.size(); }
    const std::vector<Joint> &GetJoints() const { return this->joints; }
    const Joint &GetJoint(int32_t index) const { return this->joints[index]; }

    size_t GetSkinJointCount() const { return this->skinJointToJoint.size(); }
    const std::vector<int32_t> &GetSkinJoints() const { return this->skinJointToJoint; }
    int32_t GetSkinJointIndex(int32_t jointIndex) const { return this->jointToSkinJoint[jointIndex]; }
    bool IsSkinJoint(int32_t jointIndex) const { return this->jointToSkinJoint[jointIndex] != -1; }

    const std::vector<DirectX::XMFLOAT4X4> &GetInverseBindMatrices() const { return this->inverseBindMatrices; }
};