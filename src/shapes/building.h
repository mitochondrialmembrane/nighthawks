#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shape.h"

class Building : public Shape
{
public:
    Building(glm::mat4 in_ctm, SceneMaterial in_mat, float width, float height, std::vector<float>* windowData);
    void updateParams(int param1, int param2) override;
    std::vector<float> generateShape() override { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeSide(glm::vec3 start, bool isX, bool isPositive);
    void makeCell(glm::vec3 start, int index);
    void makeFace(glm::vec3 bottomLeft, glm::vec3 h_vec, glm::vec3 v_vec);

    std::vector<float> m_vertexData;
    int m_param1;
    int columns;
    int rows;
    float window_height = 0.9782;
    float window_width = 0.5;
    float offset_x;
    float offset_y;
    float start_x;
    float start_y;
    float height;
    float width;
    std::vector<float>* window_data;
};
