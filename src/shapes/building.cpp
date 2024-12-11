#include "building.h"
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <cmath>

Building::Building(glm::mat4 in_ctm, SceneMaterial in_mat, float in_width, float in_height, std::vector<float>* windowData) : Shape(in_ctm, in_mat) {
    m_vertexData = std::vector<float>();
    columns = floor(in_width / 1.2);
    rows = floor(in_height / 2);

    start_x = (in_width - columns * window_width) / (columns + 1);
    start_y = (in_height - rows * window_height) / (rows + 1);
    offset_x = start_x + window_width;
    offset_y = start_y + window_height;
    width = in_width;
    height = in_height;
    window_data = windowData;

    makeSide(glm::vec3(0, 0, 0), false, true);
    makeSide(glm::vec3(0, 0, width), true, true);
    makeSide(glm::vec3(width, 0, width), false, false);
    makeSide(glm::vec3(width, 0, 0), true, false);
    makeFace(glm::vec3(0, height, 0), glm::vec3(0,0,width), glm::vec3(width,0,0));
}

void Building::makeSide(glm::vec3 start, bool isX, bool isPositive) {
    glm::vec3 h_vec;
    if (isX) {
        h_vec = glm::vec3(1,0,0);
    }
    else {
        h_vec = glm::vec3(0,0,1);
    }
    glm::vec3 v_vec(0, offset_y, 0);
    if (!isPositive) h_vec *= -1;

    glm::vec3 offset_start = start_x * h_vec + glm::vec3(0, start_y,0);
    makeFace(start, start_x * h_vec, glm::vec3(0, height, 0));
    for (int i = 0; i < columns; i++) {
        makeFace(start + (i + 1) * offset_x * h_vec, start_x * h_vec, glm::vec3(0, height, 0));
        glm::vec3 curr_col_start = start + ((i + 1) * offset_x - window_width) * h_vec;
        makeFace(curr_col_start, window_width * h_vec, glm::vec3(0, start_y, 0));
        for (int j = 0; j < rows; j++) {
            makeFace(curr_col_start + glm::vec3(0, (j + 1) * offset_y, 0), window_width * h_vec, glm::vec3(0, start_y, 0));
            makeCell(curr_col_start + glm::vec3(0, (j + 1) * offset_y - window_height, 0), (int) isX + 2 * (int) isPositive);
        }
    }
}

void Building::makeCell(glm::vec3 start, int index) {
    std::vector<float> data = window_data[index];
    for (int i = 0; i < data.size() / 6; i++) {
        m_vertexData.push_back(data[6 * i] + start[0]);
        m_vertexData.push_back(data[6 * i + 1] + start[1]);
        m_vertexData.push_back(data[6 * i + 2] + start[2]);
        m_vertexData.push_back(data[6 * i + 3]);
        m_vertexData.push_back(data[6 * i + 4]);
        m_vertexData.push_back(data[6 * i + 5]);
    }
}

void Building::makeFace(glm::vec3 bottomLeft, glm::vec3 h_vec, glm::vec3 v_vec) {
    glm::vec3 topLeft = bottomLeft + v_vec;
    glm::vec3 topRight = bottomLeft + h_vec + v_vec;
    glm::vec3 bottomRight = bottomLeft + h_vec;
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

void Building::updateParams(int param1, int param2) {}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Building::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
