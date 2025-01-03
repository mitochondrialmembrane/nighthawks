#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shape.h"

class Cube : public Shape
{
public:
    Cube(glm::mat4 in_ctm, SceneMaterial in_mat) : Shape(in_ctm, in_mat) {};
    void updateParams(int param1, int param2) override;
    std::vector<float> generateShape() override { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeFace(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);

    std::vector<float> m_vertexData;
    int m_param1;
};
