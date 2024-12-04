#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shape.h"

class Mesh : public Shape
{
public:
    Mesh(glm::mat4 in_ctm, SceneMaterial in_mat, std::string meshFile);
    void parseFile(std::string meshFile,
                   std::vector<glm::vec3>& faces,
                   std::vector<glm::vec3>& vertices,
                   std::vector<glm::vec3>& normals,
                   std::vector<glm::vec3>& normal_indices);
    void populateTriangles(std::vector<glm::vec3>& faces,
                           std::vector<glm::vec3>& vertices,
                           std::vector<glm::vec3>& normals,
                           std::vector<glm::vec3>& normal_indices);
    void updateParams(int param1, int param2) override;
    std::vector<float> generateShape() override { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);

    std::vector<float> m_vertexData;
    int m_param1;
};
