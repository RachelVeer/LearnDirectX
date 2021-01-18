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
const float SENSITIVITY = 0.2f;

class Camera
{
    public:
    // Camera attributes.
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Front;
    DirectX::XMFLOAT3 Up;
    DirectX::XMFLOAT3 Right;
    DirectX::XMFLOAT3 WorldUp;
    // Euler angles.
    float Yaw;
    float Pitch;
    // Camera Options
    float MovementSpeed;
    float MouseSensitiviy;

    // Constructor with vectors.
    Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 up, DirectX::XMFLOAT3 front, float yaw = YAW, float pitch = PITCH)
    : Position(position), Up(up), Front(front), Right(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)), Yaw(yaw), Pitch(pitch), MovementSpeed(SPEED), MouseSensitiviy(SENSITIVITY)
    {
        WorldUp = up;
        UpdateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles & LookAtLH Matrix.
    DirectX::XMMATRIX GetViewMatrix()
    {
        return DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&Position), DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&Position), DirectX::XMLoadFloat3(&Front)), DirectX::XMLoadFloat3(&Up));
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
            Position.x += Right.x * velocity;
            Position.y += Right.y * velocity;
            Position.z += Right.z * velocity;
        }
        if(direction == RIGHT)
        {
            Position.x -= Right.x * velocity;
            Position.y -= Right.y * velocity;
            Position.z -= Right.z * velocity;
        }
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitiviy;
        yoffset *= MouseSensitiviy;
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

        // Also re-calculate the Right and Up vector. 
        // Vectors required to perform math operations.
        DirectX::XMVECTOR right;
        DirectX::XMVECTOR camFront = DirectX::XMLoadFloat3(&Front);
        DirectX::XMVECTOR worldUp = DirectX::XMLoadFloat3(&WorldUp);
        right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(camFront, worldUp));
        // Sum of vector operation stored in float.
        DirectX::XMStoreFloat3(&Right, right);

        DirectX::XMVECTOR up;
        up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&Right), DirectX::XMLoadFloat3(&Front)));
        DirectX::XMStoreFloat3(&Up, up);
    }

};