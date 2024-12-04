#include "Cylinder.h"
#include <iostream>

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = fmax(param1, 1);
    m_param2 = fmax(param2, 3);
    setVertexData();
}

/**
 * Creates a tile on one of the bases of the cylinder
 */
void Cylinder::makeBaseTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    glm::vec3 normal = glm::normalize(glm::cross(bottomRight - bottomLeft, topLeft - bottomLeft));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);


    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
}

/**
 * Creates a tile on the side of the cylinder
 */
void Cylinder::makeSideTile(glm::vec3 topLeft,
                   glm::vec3 topRight,
                   glm::vec3 bottomLeft,
                   glm::vec3 bottomRight) {
    glm::vec3 leftNormal = glm::normalize(glm::vec3(topLeft[0], 0, topLeft[2]));
    glm::vec3 rightNormal = glm::normalize(glm::vec3(topRight[0], 0, topRight[2]));

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, leftNormal);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, leftNormal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, rightNormal);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, rightNormal);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, rightNormal);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, leftNormal);
}

void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    float rStep = 0.5f / m_param1;
    float yStep = 1.f / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float r_1 = rStep * i;
        float r_2 = rStep * (i + 1);
        float y_1 = -0.5 + yStep * i;
        float y_2 = -0.5 + yStep * (i + 1);

        // create base tiles of wedge
        makeBaseTile(
            glm::vec3(r_1 * cos(nextTheta), 0.5, r_1 * sin(nextTheta)),
            glm::vec3(r_1 * cos(currentTheta), 0.5, r_1 * sin(currentTheta)),
            glm::vec3(r_2 * cos(nextTheta), 0.5, r_2 * sin(nextTheta)),
            glm::vec3(r_2 * cos(currentTheta), 0.5, r_2 * sin(currentTheta)));

        makeBaseTile(
            glm::vec3(r_1 * cos(currentTheta), -0.5, r_1 * sin(currentTheta)),
            glm::vec3(r_1 * cos(nextTheta), -0.5, r_1 * sin(nextTheta)),
            glm::vec3(r_2 * cos(currentTheta), -0.5, r_2 * sin(currentTheta)),
            glm::vec3(r_2 * cos(nextTheta), -0.5, r_2 * sin(nextTheta)));

        makeSideTile(
            glm::vec3(0.5 * cos(nextTheta), y_2, 0.5 * sin(nextTheta)),
            glm::vec3(0.5 * cos(currentTheta), y_2, 0.5 * sin(currentTheta)),
            glm::vec3(0.5 * cos(nextTheta), y_1, 0.5 * sin(nextTheta)),
            glm::vec3(0.5 * cos(currentTheta), y_1, 0.5 * sin(currentTheta)));
    }
}

void Cylinder::setVertexData() {
    float thetaStep = 2 * M_PI / m_param2;

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = thetaStep * i;
        float nextTheta = thetaStep * (i + 1);

        makeWedge(currentTheta, nextTheta);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
