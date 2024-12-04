#version 330 core

layout(location = 0) in vec3 objPos;
layout(location = 1) in vec3 objNormal;

out vec3 worldPos;
out vec3 normal;

uniform mat4 model;

uniform mat4 view;
uniform mat4 proj;

void main() {
    worldPos = vec3(model * vec4(objPos, 1.0));
    normal = vec3(inverse(transpose(model)) * vec4(normalize(objNormal), 1.0));

    gl_Position = proj * (view * (model * vec4(objPos, 1.0)));
}
