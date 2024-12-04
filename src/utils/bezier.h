#pragma once

#include <glm/glm.hpp>
#include "../utils/scenedata.h"

class Bezier
{
public:
    Bezier();
    glm::vec3 incrementTime(float deltaT);
    glm::vec3 calculateBezier();
    void updateControls();
private:
    glm::vec3 c1;
    glm::vec3 c2;
    glm::vec3 c3;
    glm::vec3 c4;
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> ctrls;
    int curr_curve;
    float t;
    glm::vec3 currPos;
};
