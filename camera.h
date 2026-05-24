#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// Extended movement options
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float ROLL = 0.0f;
const float SPEED = 6.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler angles
    float Yaw;
    float Pitch;
    float Roll;   // now included

    // Options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 6.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW, float pitch = PITCH, float roll = ROLL)
        : Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch), Roll(roll),
        MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        updateCameraVectors();
    }

    // Returns the view matrix
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Keyboard movement (WASD + E/R)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;      // absolute up/down
        if (direction == DOWN)
            Position -= WorldUp * velocity;
    }

    // Mouse look – updates yaw and pitch (no roll)
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // Continuous rotation (used for X,Y,Z keys)
    void ProcessRotation(float pitchOffset, float yawOffset, float rollOffset) {
        Pitch += pitchOffset;
        Yaw += yawOffset;
        Roll += rollOffset;

        // Clamp pitch
        if (Pitch > 89.0f)  Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;

        // Normalize roll to [-180, 180]
        if (Roll > 180.0f)  Roll -= 360.0f;
        if (Roll < -180.0f) Roll += 360.0f;

        updateCameraVectors();
    }

    // Make the camera look at a specific target (used in orbit mode)
    void LookAt(const glm::vec3& target) {
        Front = glm::normalize(target - Position);

        // Compute approximate yaw/pitch from Front
        Yaw = glm::degrees(atan2(Front.z, Front.x));
        Pitch = glm::degrees(asin(Front.y));

        // Recompute Right and Up (preserving current roll)
        updateCameraVectors();
    }

    // Mouse scroll
    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)  Zoom = 1.0f;
        if (Zoom > 45.0f) Zoom = 45.0f;
    }

private:
    // Recalculate Front, Right, Up from Yaw, Pitch, Roll
    void updateCameraVectors() {
        // Base front from yaw & pitch
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // Right and Up without roll (using world up)
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));

        // Apply roll: rotate Right and Up around Front
        if (Roll != 0.0f) {
            float rad = glm::radians(Roll);
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rad, Front);
            Right = glm::normalize(glm::vec3(rot * glm::vec4(Right, 0.0f)));
            Up = glm::normalize(glm::cross(Right, Front));
        }
    }
};

#endif