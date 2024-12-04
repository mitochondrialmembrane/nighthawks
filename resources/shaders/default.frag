#version 330 core
in vec3 worldPos;
in vec3 normal;

struct Light {
    int type;

    vec3 color;
    vec3 function; // Attenuation function
    vec3 dir;      // Not applicable to point lights
    vec3 pos;

    float penumbra; // Only applicable to spot lights, in RADIANS
    float angle;    // Only applicable to spot lights, in RADIANS
};

// ambient lighting
uniform vec3 cAmbient;
uniform float k_a;

// diffuse lighting
uniform vec3 cDiffuse;
uniform Light lights[8];
uniform int numLights;
uniform float k_d;

// specular lighting
uniform vec3 cSpecular;
uniform vec4 camPos;
uniform float shininess;
uniform float k_s;

out vec4 color;

void main() {
    color = vec4(k_a * cAmbient, 1.0);

    vec3 lightDir;
    float atten;
    for (int i = 0; i < numLights; i++) {
        Light light = lights[i];

        switch (light.type) {
        // point light
        case 0:
            lightDir = normalize(light.pos - worldPos);
            float distance = length(light.pos - worldPos);
            atten = min(1, 1 / (light.function[0] + distance * (light.function[1] + distance * light.function[2])));
            break;
        // directional light
        case 1:
            lightDir = -normalize(light.dir);
            atten = 1;
            break;
        // spot light
        case 2:
            lightDir = normalize(light.pos - worldPos);
            float angle = acos(dot(-lightDir, normalize(light.dir)));

            if (angle <= light.angle) {
                float distance = length(light.pos - worldPos);
                atten = min(1, 1 / (light.function[0] + distance * (light.function[1] + distance * light.function[2])));

                if (angle > light.angle - light.penumbra) {
                    float n = (angle - light.angle) / light.penumbra + 1;
                    atten *= 1 - (-2 * pow(n, 3) + 3 * pow(n, 2));
                }
            }
            else atten = 0;
            break;
        default:
            break;
        }

        float diffuse = k_d * max(dot(lightDir, normalize(normal)), 0);
        vec3 camDir = normalize(vec3(camPos) - worldPos);
        vec3 reflectDir = reflect(-lightDir, normalize(normal));
        float specular = k_s * pow(max(dot(camDir, reflectDir), 0), max(shininess, 0.0001));

        for (int j = 0; j < 3; j++) {
            color[j] += (diffuse * cDiffuse[j] + specular * cSpecular[j]) * light.color[j] * atten;
        }
    }

}
