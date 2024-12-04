#pragma once
#include <vector>
#include "shapes/shape.h"
#include <QOpenGLWidget>

namespace RealtimeUtils {

void clearArrays(std::vector<Shape*> shapes, QOpenGLWidget *widget);
void buildArrays(std::vector<Shape*> shapes, QOpenGLWidget *widget, int shapeParameter1, int shapeParameter2);

}
