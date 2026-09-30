#include "free_cam.hpp"
#include "components/transform_component.hpp"
#include "input/input.hpp"

void FreeCam::Update(float deltaTime)
{
    this->UpdateRotation(deltaTime);
    this->UpdateMovement(deltaTime);
}

void FreeCam::UpdateRotation(float deltaTime)
{
    if (Input::IsMouseButtonDown(MouseButton::RIGHT))
    {
        Input::SetMouseMode(MouseMode::CAPTURED);

        XMFLOAT2 delta = Input::GetMouseDelta();

        this->rotation.x += delta.y * MOUSE_SENSITIVITY;

        if (this->rotation.x > XM_PIDIV2)
            rotation.x = XM_PIDIV2;
        else if (this->rotation.x < -XM_PIDIV2)
            rotation.x = -XM_PIDIV2;

        this->rotation.y += delta.x * MOUSE_SENSITIVITY;
        
        float wheel = Input::GetMouseWheelY();
        this->speed += wheel * WHEEL_SENSITIVITY;
        if (this->speed > MAX_SPEED)
            this->speed = MAX_SPEED;
        else if (this->speed < MIN_SPEED)
            this->speed = MIN_SPEED;
    }
    else
    {
        Input::SetMouseMode(MouseMode::VISIBLE);
    }
}

void FreeCam::UpdateMovement(float deltaTime)
{
    XMVECTOR quat = XMQuaternionRotationRollPitchYaw(
        this->rotation.x,
        this->rotation.y,
        this->rotation.z
    );

    XMVECTOR right = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), quat);
    XMVECTOR forward = XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), quat);

    XMVECTOR direction = XMVectorSet(0, 0, 0, 0);

    if (Input::IsKeyDown(Key::A))
    {
        direction = XMVectorAdd(direction, -right);
    }

    if (Input::IsKeyDown(Key::D))
    {
        direction = XMVectorAdd(direction, right);
    }

    if (Input::IsKeyDown(Key::W))
    {
        direction = XMVectorAdd(direction, forward);
    }

    if (Input::IsKeyDown(Key::S))
    {
        direction = XMVectorAdd(direction, -forward);
    }

    if (Input::IsKeyDown(Key::SPACE) || Input::IsKeyDown(Key::Q))
    {
        direction = XMVectorAdd(direction, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    }

    if (Input::IsKeyDown(Key::LEFT_CONTROL) || Input::IsKeyDown(Key::E))
    {
        direction = XMVectorAdd(direction, XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
    }

    XMVECTOR newTargetPosition = XMVectorAdd(
        XMLoadFloat3(&this->targetPosition),
        XMVectorScale(direction, this->speed * deltaTime)
    );

    XMStoreFloat3(&this->targetPosition, newTargetPosition);

    XMVECTOR newPosition = XMVectorLerp(
        XMLoadFloat3(&this->position),
        XMLoadFloat3(&this->targetPosition),
        deltaTime * this->POSITION_LERP_SPEED
    );

    XMStoreFloat3(&this->position, newPosition);
}

XMMATRIX FreeCam::GetViewMatrix()
{
    XMMATRIX transform = XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
    transform *= XMMatrixTranslation(this->position.x, this->position.y, this->position.z);

    XMMATRIX viewMatrix = XMMatrixInverse(nullptr, transform);

    return viewMatrix;
}
