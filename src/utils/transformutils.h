#pragma once
#include <glm/glm.hpp>

namespace TransformUtils {

glm::mat4 translate(glm::vec3 offset);
glm::mat4 rotate(glm::vec3 axis, float angle);

}
