#include "bezier.h"
#include <iostream>

Bezier::Bezier() :
    curr_curve(0),
    t(0)
{
    currPos = glm::vec3();

    // points.push_back(glm::vec3(0, 0, 0));
    // points.push_back(glm::vec3(2, 0, 2));
    // points.push_back(glm::vec3(0, 0, 4));
    // points.push_back(glm::vec3(-2, 0, 2));
    // ctrls.push_back(glm::vec3(1.2, 0, 0));
    // ctrls.push_back(glm::vec3(0, 0, 1.2));
    // ctrls.push_back(glm::vec3(-1.2, 0, 0));
    // ctrls.push_back(glm::vec3(0, 0, -1.2));
    points.clear();
    ctrls.clear();

    points.push_back(glm::vec3(15, 15, 15));  // start far
    points.push_back(glm::vec3(12, 12, 12));
    points.push_back(glm::vec3(6, 3, 6));
    points.push_back(glm::vec3(2, 0, 2));     // final zoom

    // controls
    ctrls.push_back(glm::vec3(-2, -1, -2));
    ctrls.push_back(glm::vec3(-2, -1, -2));
    ctrls.push_back(glm::vec3(-1, -1, -1));
    ctrls.push_back(glm::vec3(-1, -1, -1));

    // circle building
    points.push_back(glm::vec3(2, -1, 0));
    points.push_back(glm::vec3(-3, -1, 4));
    points.push_back(glm::vec3(-5, -1, 2));
    points.push_back(glm::vec3(-7, -1, -5));
    points.push_back(glm::vec3(-9, -1, -7));
    points.push_back(glm::vec3(-10, -1, -7));


    // points.push_back(glm::vec3(-7, 0.2, -8));
    // points.push_back(glm::vec3(2, 0.2, 2));

    // More natural circular motion control points
    ctrls.push_back(glm::vec3(0, 0, 0));
    ctrls.push_back(glm::vec3(-2, 0, 0));
    ctrls.push_back(glm::vec3(-2, 0, 0));
    ctrls.push_back(glm::vec3(-1, 0, -1));
    ctrls.push_back(glm::vec3(-1, 0, -1));
    ctrls.push_back(glm::vec3(-1, 0, 0));




    // ctrls.push_back(glm::vec3(-2, 0, -5.f));

    // ctrls.push_back(glm::vec3(-2, 0, 2));
    // ctrls.push_back(glm::vec3(-2, 0, 5));
    // ctrls.push_back(glm::vec3(-2, 0, 3));
    // ctrls.push_back(glm::vec3(-3, 0, 2));
    // ctrls.push_back(glm::vec3(2, 1, 0));    // ret


    updateControls();
}

glm::vec3 Bezier::incrementTime(float deltaT) {
    t += deltaT;
    if (t > 1) updateControls();

    glm::vec3 lastPos = currPos;
    currPos = calculateBezier();
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
