#include "skeleton.hpp"
#include "core/logging.hpp"

bool Skeleton::Create(
    const std::vector<Joint> &joints,
    const std::vector<int32_t> &skinJointToJoint,
    const std::vector<XMFLOAT4X4> &inverseBindMatrices
) {
    // CONTINUE HERE!

    return true;
}

int32_t Skeleton::FindJoint(const std::string &name) const {
    auto iter = this->jointIndeces.find(name);
    return iter != this->jointIndeces.end() ? iter->second : -1;
}