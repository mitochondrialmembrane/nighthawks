#include "Cube.h"
#include <iostream>

void Cube::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
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

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    glm::vec3 w = (1.f / m_param1) * (topRight - topLeft);
    glm::vec3 h = (1.f / m_param1) * (topLeft - bottomLeft);


    for (int i = 0; i < m_param1; i++) {
        for (int j = 0; j < m_param1; j++) {
            float x = i;
            float y = j;
            makeTile(bottomLeft + x * w + (y + 1) * h,
                     bottomLeft + (x + 1) * w + (y + 1) * h,
                     bottomLeft + x * w + y * h,
                     bottomLeft + (x + 1) * w + y * h);
        }
    }

}

void Cube::setVertexData() {

     makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
              glm::vec3( 0.5f,  0.5f, 0.5f),
              glm::vec3(-0.5f, -0.5f, 0.5f),
              glm::vec3( 0.5f, -0.5f, 0.5f));

    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3(0.5f, -0.5f,  0.5f),
              glm::vec3(0.5f,  0.5f,  0.5f),
              glm::vec3(0.5f, -0.5f, -0.5f),
              glm::vec3(0.5f,  0.5f, -0.5f));

    makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
              glm::vec3( 0.5f,  0.5f,  0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f,  0.5f,  0.5f));

    makeFace(glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
