#include "Sphere.h"
#include <iostream>

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = fmax(param1, 2);
    m_param2 = fmax(param2, 3);
    setVertexData();
}

/**
 * Creates one rectangular tile in the sphere
 */
void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(bottomLeft));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(topRight));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));
}

/**
 * Creates one vertical wedge of the sphere
 */
void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float phiStep = M_PI / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float phi_1 = phiStep * i;
        float phi_2 = phiStep * (i + 1);

        makeTile(
            glm::vec3(0.5 * sin(phi_1) * cos(nextTheta), 0.5 * cos(phi_1), 0.5 * sin(phi_1) * sin(nextTheta)),
            glm::vec3(0.5 * sin(phi_1) * cos(currentTheta), 0.5 * cos(phi_1), 0.5 * sin(phi_1) * sin(currentTheta)),
            glm::vec3(0.5 * sin(phi_2) * cos(nextTheta), 0.5 * cos(phi_2), 0.5 * sin(phi_2) * sin(nextTheta)),
            glm::vec3(0.5 * sin(phi_2) * cos(currentTheta), 0.5 * cos(phi_2), 0.5 * sin(phi_2) * sin(currentTheta)));
    }
}

void Sphere::makeSphere() {
    float thetaStep = 2 * M_PI / m_param2;

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = thetaStep * i;
        float nextTheta = thetaStep * (i + 1);

        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
