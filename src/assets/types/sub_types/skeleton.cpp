#include "skeleton.hpp"
#include "core/logging.hpp"

bool Skeleton::Create(
    const std::vector<Joint> &joints,
    const std::vector<int32_t> &skinJointToJoint,
    const std::vector<XMFLOAT4X4> &inverseBindMatrices
) {
    if (skinJointToJoint.size() != inverseBindMatrices.size()) {
        LogWarn("Skeleton has '%zu' skin joints but '%zu' inverse bind matrices (should be equal)", skinJointToJoint.size(), inverseBindMatrices.size());
        return false;
    }

    for (size_t i = 0; i < joints.size(); ++i) {
        if (joints[i].parentIndex >= static_cast<int32_t>(i)) {
            LogWarn("Joint '%s' comes before its parent", joints[i].name.c_str());
            return false;
        }
    }

    this->joints = joints;
    this->skinJointToJoint = skinJointToJoint;
    this->inverseBindMatrices = inverseBindMatrices;

    this->jointToSkinJoint.assign(joints.size(), -1);
    for (size_t i = 0; i < skinJointToJoint.size(); ++i) {
        const int32_t jointIndex = skinJointToJoint[i];
        if (jointIndex < 0 || jointIndex >= static_cast<int32_t>(joints.size())) {
            LogWarn("Invalid skin joint reference");
            return false;
        }

        this->jointToSkinJoint[jointIndex] = static_cast<int32_t>(i);
    }

    this->jointIndices.clear();
    for (size_t i = 0; i < joints.size(); ++i)
        if (!this->jointIndices.emplace(joints[i].name, static_cast<int32_t>(i)).second)
            LogWarn("Skeleton has more than one joint named '%s'; only the first can be targeted", joints[i].name.c_str());

    return true;
}

int32_t Skeleton::FindJoint(const std::string &name) const {
    auto iter = this->jointIndices.find(name);
    return iter != this->jointIndices.end() ? iter->second : -1;
}