#include "free_cam.hpp"

#include "components/transform_component.hpp"

XMMATRIX FreeCam::GetViewMatrix()
{
    XMMATRIX transform = XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
    transform *= XMMatrixTranslation(this->position.x, this->position.y, this->position.z);

    XMMATRIX viewMatrix = XMMatrixInverse(nullptr, transform);

    return viewMatrix;
}

void FreeCam::SetTransformMatrix(XMMATRIX matrix)
{
    this->position = TransformComponent::ExtractTranslation(matrix);
    
    XMFLOAT4 quat = TransformComponent::ExtractRotation(matrix);
    this->rotation = TransformComponent::QuaternionToEulerAngles(quat);
}
