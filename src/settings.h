#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath = "scenefiles/illuminate/required/point_light/point_light_2.json";
    int shapeParameter1 = 1;
    int shapeParameter2 = 1;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // doesn't matter for us I think
    bool perPixelFilter = false;
    bool kernelBasedFilter = false;
    bool extraCredit1 = false;
    bool extraCredit2 = false;
    bool extraCredit3 = false;
    bool extraCredit4 = false;
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H
