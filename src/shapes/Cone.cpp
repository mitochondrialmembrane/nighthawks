#include "Cone.h"
#include <iostream>

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = fmax(param1, 1);
    m_param2 = fmax(param2, 3);
    setVertexData();
}

/**
 * Creates a tile on the bottom face of the cone
 */
void Cone::makeBaseTile(glm::vec3 topLeft,
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
 * Creates a tile on the conical side of the cone, with the given points and normals
 */
void Cone::makeSideTile(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight,
                        glm::vec3 topLeftNormal,
                        glm::vec3 topRightNormal,
                        glm::vec3 bottomLeftNormal,
                        glm::vec3 bottomRightNormal) {

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, topLeftNormal);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bottomLeftNormal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, bottomRightNormal);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, bottomRightNormal);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, topRightNormal);
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, topLeftNormal);
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    float rStep = 0.5f / m_param1;
    float yStep = 1.f / m_param1;

    glm::vec3 leftNormal = glm::normalize(glm::vec3(cos(nextTheta), 0.5, sin(nextTheta)));
    glm::vec3 rightNormal = glm::normalize(glm::vec3(cos(currentTheta), 0.5, sin(currentTheta)));

    for (int i = 0; i < m_param1; i++) {
        float r_1 = rStep * i;
        float r_2 = rStep * (i + 1);
        float y_1 = -0.5 + yStep * i;
        float y_2 = -0.5 + yStep * (i + 1);

        makeBaseTile(
            glm::vec3(r_1 * cos(currentTheta), -0.5, r_1 * sin(currentTheta)),
            glm::vec3(r_1 * cos(nextTheta), -0.5, r_1 * sin(nextTheta)),
            glm::vec3(r_2 * cos(currentTheta), -0.5, r_2 * sin(currentTheta)),
            glm::vec3(r_2 * cos(nextTheta), -0.5, r_2 * sin(nextTheta)));

        // case for the tip
        if (i == m_param1 - 1) {
            glm::vec3 tipNormal = 0.5f * (leftNormal + rightNormal);
            makeSideTile(
                glm::vec3((0.5 - r_2) * cos(nextTheta), y_2, (0.5 - r_2) * sin(nextTheta)),
                glm::vec3((0.5 - r_2) * cos(currentTheta), y_2, (0.5 - r_2) * sin(currentTheta)),
                glm::vec3((0.5 - r_1) * cos(nextTheta), y_1, (0.5 - r_1) * sin(nextTheta)),
                glm::vec3((0.5 - r_1) * cos(currentTheta), y_1, (0.5 - r_1) * sin(currentTheta)),
                tipNormal, tipNormal, leftNormal, rightNormal);
        }
        // other side tiles
        else {
            makeSideTile(
                glm::vec3((0.5 - r_2) * cos(nextTheta), y_2, (0.5 - r_2) * sin(nextTheta)),
                glm::vec3((0.5 - r_2) * cos(currentTheta), y_2, (0.5 - r_2) * sin(currentTheta)),
                glm::vec3((0.5 - r_1) * cos(nextTheta), y_1, (0.5 - r_1) * sin(nextTheta)),
                glm::vec3((0.5 - r_1) * cos(currentTheta), y_1, (0.5 - r_1) * sin(currentTheta)),
                leftNormal, rightNormal, leftNormal, rightNormal);
        }
    }
}

void Cone::setVertexData() {
    float thetaStep = 2 * M_PI / m_param2;

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = thetaStep * i;
        float nextTheta = thetaStep * (i + 1);

        makeWedge(currentTheta, nextTheta);
    }
}


// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
