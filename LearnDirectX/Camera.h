#pragma once

#include <DirectXMath.h>
#include <cmath>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values.
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.5f;

class Camera
{
    public:
    // Camera attributes.
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Front;
    DirectX::XMFLOAT3 Up;
    DirectX::XMFLOAT3 FinalPos;
    // Euler angles.
    float Yaw;
    float Pitch;
    // Camera Options
    float MovementSpeed;

    // Constructor with vectors.
    Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 up, DirectX::XMFLOAT3 front, float yaw = YAW, float pitch = PITCH)
    : Position(position), Up(up), Front(up), FinalPos(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)), Yaw(yaw), Pitch(pitch), MovementSpeed(SPEED)
    {
        UpdateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles & LookAtLH Matrix.
    DirectX::XMMATRIX GetViewMatrix()
    {
        DirectX::XMVECTOR camPos = XMLoadFloat3(&Position);
        DirectX::XMVECTOR camFront = XMLoadFloat3(&Front);
        DirectX::XMVECTOR camUp = XMLoadFloat3(&Up);
        return DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, camFront), camUp);
    }

    void ProcessKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if(direction == FORWARD)
        {
            Position.x += Front.x * velocity;
            Position.y += Front.y * velocity;
            Position.z += Front.z * velocity;
        }
        if(direction == BACKWARD)
        {
            Position.x -= Front.x * velocity;
            Position.y -= Front.y * velocity;
            Position.z -= Front.z * velocity;
        }
        if(direction == LEFT)
        {
            // Vectors required to perform math operations.
            DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&Position);
            DirectX::XMVECTOR camFront = DirectX::XMLoadFloat3(&Front);
            DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&Up);
            camPos = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(camFront, camUp));
            // Sum of vector operation stored in float.
            DirectX::XMStoreFloat3(&FinalPos, camPos);
            // 'finalPos' float now serves it purposes to adjust the actual camera floating points/coords.
            Position.x += FinalPos.x * velocity;
            Position.y += FinalPos.y * velocity;
            Position.z += FinalPos.z * velocity;
        }
        if(direction == RIGHT)
        {
            // Vectors required to perform math operations.
            DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&Position);
            DirectX::XMVECTOR camFront = DirectX::XMLoadFloat3(&Front);
            DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&Up);
            camPos = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(camFront, camUp));
            // Sum of vector operation stored in float.
            DirectX::XMStoreFloat3(&FinalPos, camPos);
            // 'finalPos' float now serves it purposes to adjust the actual camera floating points/coords.
            Position.x -= FinalPos.x * velocity;
            Position.y -= FinalPos.y * velocity;
            Position.z -= FinalPos.z * velocity;
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        float sensitivity = 0.3f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;
        Yaw -= xoffset;
        Pitch -= yoffset;
        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
        
        UpdateCameraVectors();
    }
    private:
    // Calculate the front vector from the Camera's (updated) Euler angle.
    void UpdateCameraVectors()
    {
        // Calculate the new Front vector.
        DirectX::XMFLOAT3 updatedFront;
        updatedFront.x = (float)cos(DirectX::XMConvertToRadians(Yaw)) * (float)cos(DirectX::XMConvertToRadians(Pitch));
        updatedFront.y = (float)sin(DirectX::XMConvertToRadians(Pitch));
        updatedFront.z = (float)sin(DirectX::XMConvertToRadians(Yaw)) * (float)cos(DirectX::XMConvertToRadians(Pitch));
        Front = updatedFront;
    }

};