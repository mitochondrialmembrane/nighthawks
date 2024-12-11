#include "camera.h"
#include <glm/gtx/transform.hpp>
#include "utils/scenedata.h"
#include <cmath>
#include <iostream>

Camera::Camera(SceneCameraData in_data, int in_width, int in_height, float in_near, float in_far) :
    data(in_data),
    width(in_width),
    height(in_height),
    viewMatrix(),
    viewMatrixInverted(),
    aspectRatio((float) width / (float) height),
    heightAngle(data.heightAngle),
    widthAngle(0),
    clipNear(in_near),
    clipFar(in_far),
    projMatrix(),
    up(data.up),
    camPos(data.pos)
{
    viewMatrix = calculateViewMatrix(data.look);
    viewMatrixInverted = glm::inverse(viewMatrix);
    widthAngle = 2 * atan(aspectRatio * tan(heightAngle / 2));
    projMatrix = calculateProjMatrix();
}

/**
 * Calculates the view matrix of the camera
 */
glm::mat4 Camera::calculateViewMatrix(glm::vec3 look) {
    glm::vec3 w = -glm::normalize(look);
    glm::vec3 v = glm::normalize(up - (glm::dot(up, w)) * w);
    glm::vec3 u = glm::cross(v, w);
    glm::mat4 rotateMatrix = glm::mat4(
        u[0], v[0], w[0], 0,
        u[1], v[1], w[1], 0,
        u[2], v[2], w[2], 0,
        0, 0, 0, 1);

    glm::mat4 translateMatrix = glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -camPos[0], -camPos[1], -camPos[2], 1);

    return rotateMatrix * translateMatrix;
}

/**
 * Calculates the perspective matrix of the camera
 */
glm::mat4 Camera::calculateProjMatrix() {
    float f = 1.f / clipFar;
    glm::mat4 scaleMatrix = glm::mat4(
        f / tan(widthAngle / 2.f), 0, 0, 0,
        0, f / tan(heightAngle / 2.f), 0, 0,
        0, 0, f, 0,
        0, 0, 0, 1.f);

    float c = -clipNear / clipFar;
    glm::mat4 unhingingMatrix = glm::mat4(
        1.f, 0,   0,                0,
        0,   1.f, 0,                0,
        0,   0,   1.f / (c + 1.f), -1.f,
        0,   0,  -c / (c + 1.f),    0);

    glm::mat4 remappingMatrix = glm::mat4(
        1.f, 0, 0, 0,
        0, 1.f, 0, 0,
        0, 0, -2.f, 0,
        0, 0, -1.f, 1.f);
    return remappingMatrix * (unhingingMatrix * scaleMatrix);
}

glm::mat4 Camera::getViewMatrix() const {
    return viewMatrix;
}

glm::mat4 Camera::getViewMatrixInverted() const {
    return viewMatrixInverted;
}

glm::mat4 Camera::getProjMatrix() const {
    return projMatrix;
}

/**
 * Updates the clipping plane values, and recalculates the projection matrix
 */
void Camera::updateClippingPlanes(float near, float far) {
    clipNear = near;
    clipFar = far;
    projMatrix = calculateProjMatrix();
}

/**
 * Updates the width and height-associated values, and recalculates the projection matrix
 */
void Camera::updateWidthHeight(int w, int h) {
    width = w;
    height = h;
    aspectRatio = (float) width / (float) height;
    widthAngle = 2 * atan(aspectRatio * tan(heightAngle / 2));
    projMatrix = calculateProjMatrix();
}

/**
 * Translate the camera by the given offset along its look vector
 */
void Camera::translate(glm::vec3 offset) {
    viewMatrix *= glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -offset[0], -offset[1], -offset[2], 1
        );
    // viewMatrix = calculateViewMatrix(glm::vec3(0,0,0));
    viewMatrixInverted = glm::inverse(viewMatrix);
    camPos = viewMatrixInverted[3];
}

/**
 * Rotates the camera about the given axis by the given angle
 */
void Camera::rotate(glm::vec3 axis, float angle) {
    float s = sin(angle), c = cos(angle), ci = 1 - c;
    glm::vec3 u = glm::normalize(axis);
    float ux = u[0], uy = u[1], uz = u[2];

    glm::vec3 look = glm::mat4(
                         c + ux * ux * ci,      ux * uy * ci - uz * s, ux * uz * ci + uy * s, 0,
                         ux * uy * ci + uz * s, c + uy * uy * ci,      uy * uz * ci - ux * s, 0,
                         ux * uz * ci - uy * s, uy * uz * ci + ux * s, c + uz * uz * ci,      0,
                         0,                     0,                     0,                     1
                         ) * -glm::vec4(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2], 0);
    viewMatrix = calculateViewMatrix(look);
    viewMatrixInverted = glm::inverse(viewMatrix);
}

float Camera::getAspectRatio() const {
    return aspectRatio;
}

float Camera::getHeightAngle() const {
    return heightAngle;
}

float Camera::getWidthAngle() const {
    return widthAngle;
}

float Camera::getFocalLength() const {
    return data.focalLength;
}

float Camera::getAperture() const {
    return data.aperture;
}

glm::vec4 Camera::getCamPos() {
    return camPos;
}

void Camera::setLookVector(const glm::vec3& look) {
    viewMatrix = calculateViewMatrix(look);
    viewMatrixInverted = glm::inverse(viewMatrix);
}
