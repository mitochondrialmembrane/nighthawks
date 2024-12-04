#include "bezier.h"
#include <iostream>

Bezier::Bezier() :
    curr_curve(0),
    t(0)
{
    currPos = glm::vec3();

    points.push_back(glm::vec3(0, 0, 0));
    points.push_back(glm::vec3(2, 0, 2));
    points.push_back(glm::vec3(0, 0, 4));
    points.push_back(glm::vec3(-2, 0, 2));
    ctrls.push_back(glm::vec3(1.2, 0, 0));
    ctrls.push_back(glm::vec3(0, 0, 1.2));
    ctrls.push_back(glm::vec3(-1.2, 0, 0));
    ctrls.push_back(glm::vec3(0, 0, -1.2));

    updateControls();
}

glm::vec3 Bezier::incrementTime(float deltaT) {
    t += deltaT;
    if (t > 1) updateControls();

    glm::vec3 lastPos = currPos;
    currPos = calculateBezier();
    std::cout << currPos[0] << "," << currPos[1] << "," << currPos[2] << "\n";
    return currPos - lastPos;
}

glm::vec3 Bezier::calculateBezier() {
    float t_flip = 1 - t;

    float t1 = pow(t_flip, 3);
    float t2 = 3 * t_flip * t_flip * t;
    float t3 = 3 * t_flip * t * t;
    float t4 = pow(t, 3);

    glm::vec3 out;
    for (int i = 0; i < 3; i++) {
        out[i] = t1 * c1[i] + t2 * c2[i] + t3 * c3[i] + t4 * c4[i];
    }
    return out;
}

void Bezier::updateControls() {
    int next_curve = (curr_curve + 1) % points.size();
    c1 = points[curr_curve];
    c2 = c1 + ctrls[curr_curve];
    c4 = points[next_curve];
    c3 = c4 - ctrls[next_curve];
    curr_curve = next_curve;
    t = 0;
}
