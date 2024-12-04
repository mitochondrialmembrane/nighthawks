#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "utils/scenedata.h"
#include <GL/glew.h>

class Shape
{
public:
    Shape(glm::mat4 in_ctm, SceneMaterial in_mat) : ctm(in_ctm), mat(in_mat) {};
    virtual void updateParams(int param1, int param2) = 0;
    virtual std::vector<float> generateShape() = 0;
    glm::mat4 getCTM() { return ctm; }
    SceneMaterial getMat() { return mat; }
    GLuint *getVBO() { return &vbo; }
    GLuint *getVAO() { return &vao; }
    virtual ~Shape() = default;

private:
    glm::mat4 ctm;
    SceneMaterial mat;
    GLuint vbo;
    GLuint vao;
};
