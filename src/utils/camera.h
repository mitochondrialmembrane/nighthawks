#pragma once

#include <glm/glm.hpp>
#include "../utils/scenedata.h"

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
public:
    Camera() {};
    Camera(SceneCameraData data, int width, int height, float in_near, float in_far);
    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.
    glm::mat4 getViewMatrix() const;

    glm::mat4 getViewMatrixInverted() const;

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the width angle of the camera in RADIANS.
    float getWidthAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;

    // returns the projection matrix of the camera
    glm::mat4 getProjMatrix() const;

    glm::vec4 getCamPos();

    void updateClippingPlanes(float near, float far);
    void updateWidthHeight(int width, int height);
    void translate(glm::vec3 offset);
    void rotate(glm::vec3 axis, float angle);
    void setLookVector(const glm::vec3& look);


private:
    SceneCameraData data;
    int width;
    int height;
    glm::mat4 viewMatrix;
    glm::mat4 viewMatrixInverted;
    glm::mat4 projMatrix;
    float heightAngle;
    float widthAngle;
    float aspectRatio;
    float clipNear;
    float clipFar;
    glm::vec4 camPos;
    glm::vec3 up;

    glm::mat4 calculateViewMatrix(glm::vec3 look);
    glm::mat4 calculateProjMatrix();
};
