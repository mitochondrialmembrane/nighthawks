#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

void dfsPopulatePrimitives(SceneNode *node, RenderData *renderData, glm::mat4 ctm) {
    std::vector<SceneTransformation*> transformations = node->transformations;

    for (int i = 0; i < transformations.size(); i++) {
        switch (transformations[i]->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm *= glm::translate(transformations[i]->translate);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm *= glm::rotate(transformations[i]->angle, transformations[i]->rotate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm *= glm::scale(transformations[i]->scale);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm = transformations[i]->matrix * ctm;
            break;
        }
    }

    for (int i = 0; i < node->primitives.size(); i++) {
        renderData->shapes.push_back(RenderShapeData{*node->primitives[i], ctm});
    }

    for (int i = 0; i < node->lights.size(); i++) {
        SceneLight light = *(node->lights[i]);
        SceneLightData lightData;
        lightData.id = light.id;
        lightData.function = light.function;
        lightData.type = light.type;
        lightData.color = light.color;

        switch (light.type) {
        case LightType::LIGHT_POINT:
            lightData.pos = ctm * glm::vec4(0, 0, 0, 1);
            break;
        case LightType::LIGHT_DIRECTIONAL:
            lightData.dir = ctm * light.dir;
            break;
        case LightType::LIGHT_SPOT:
            lightData.pos = ctm * glm::vec4(0, 0, 0, 1);
            lightData.dir = ctm * light.dir;
            lightData.penumbra = light.penumbra;
            lightData.angle = light.angle;
            break;
        }
        renderData->lights.push_back(lightData);
    }

    for (int i = 0; i < node->children.size(); i++) {
        dfsPopulatePrimitives(node->children[i], renderData, ctm);
    }
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();

    renderData.shapes.clear();
    dfsPopulatePrimitives(fileReader.getRootNode(), &renderData, glm::mat4(1.0));

    return true;
}
