#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include "shapes/Cone.h"
#include "shapes/Cube.h"
#include "shapes/Cylinder.h"
#include "shapes/Sphere.h"
#include "shapes/mesh.h"
#include "shapes/building.h"
#include "utils/shaderloader.h"
#include "utils/realtimeutils.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"


// ================== Project 5: Lights, Camera

/**
 * @brief Realtime::Realtime constructor for the Realtime class
 */
Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
}

/**
 * @brief Realtime::finish called upon program close --- frees all allocated memory
 */
void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // clear all VAOs, VBOs for shapes in our scene
    RealtimeUtils::clearArrays(shapes, this);

    // delete all allocated memory
    for (Shape *shape : shapes) {
        delete shape;
    }

    // delete shaders, additional VAOs, FBO
    glDeleteProgram(m_shader);
    glDeleteProgram(m_texture_shader);
    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    this->doneCurrent();
}

/**
 * @brief Realtime::initializeGL handles initialization and calls function that loads our scene
 */
void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    // set background color
    glClearColor(0.031, 0.122, 0.114, 1.0f);

    // start timer
    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // set-up our FBO
    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Initializing GL : GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // draw objects appropriately on top of one another, cull faces non-visible faces, set dimensions
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, m_screen_width, m_screen_height);

    // load our shaders
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    // set up some texture-related stuff
    glUseProgram(m_texture_shader);
    glUniform1f(glGetUniformLocation(m_texture_shader, "tex"), 0);
    glUseProgram(0);
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    //
            -1.f,  1.f, 0.0f, 0, 1,
            -1.f, -1.f, 0.0f, 0, 0,
            1.f, -1.f, 0.0f, 1, 0,
            1.f,  1.f, 0.0f, 1, 1,
            -1.f,  1.f, 0.0f, 0, 1,
            1.f, -1.f, 0.0f, 1, 0
        };

    // generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // call makeFBO
    makeFBO();

    // call sceneChanged to actually parse our scene
    sceneChanged();
}

/**
 * @brief Realtime::paintGL called whenever openGL state changes --- actually creates visuals
 */
void Realtime::paintGL() {
    // clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind our main (phong) shader
    glUseProgram(m_shader);

    // bind our frame buffer and do some basic setup
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // send camera info to the shader
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"), 1, GL_FALSE, &(camera.getViewMatrix())[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "proj"), 1, GL_FALSE, &(camera.getProjMatrix())[0][0]);
    glUniform4fv(glGetUniformLocation(m_shader, "camPos"), 1, &(camera.getCamPos())[0]);

    // send light info
    for (int i = 0; i < numLights; i++) {
        SceneLightData light = lights[i];

        // get all names set-up
        std::string baseName = "lights[" + std::to_string(i) + "]";
        GLuint typeLoc = glGetUniformLocation(m_shader, (baseName + ".type").c_str());
        GLuint colorLoc = glGetUniformLocation(m_shader, (baseName + ".color").c_str());
        GLuint dirLoc = glGetUniformLocation(m_shader, (baseName + ".dir").c_str());
        GLuint functionLoc = glGetUniformLocation(m_shader, (baseName + ".function").c_str());
        GLuint posLoc = glGetUniformLocation(m_shader, (baseName + ".pos").c_str());

        // light color info is sent
        glUniform3f(colorLoc, light.color[0], light.color[1], light.color[2]);
        glUniform3f(dirLoc, light.dir[0], light.dir[1], light.dir[2]);
        glUniform3f(functionLoc, light.function[0], light.function[1], light.function[2]);
        glUniform3f(posLoc, light.pos[0], light.pos[1], light.pos[2]);

        // type-related info is sent
        switch (light.type) {
        case LightType::LIGHT_POINT:
            glUniform1i(typeLoc, 0);
            break;
        case LightType::LIGHT_DIRECTIONAL:
            glUniform1i(typeLoc, 1);
            break;
        case LightType::LIGHT_SPOT:
            glUniform1i(typeLoc, 2);
            GLuint penumbraLoc = glGetUniformLocation(m_shader, (baseName + ".penumbra").c_str());
            GLuint angleLoc = glGetUniformLocation(m_shader, (baseName + ".angle").c_str());
            glUniform1f(penumbraLoc, light.penumbra);
            glUniform1f(angleLoc, light.angle);
            break;
        }
    }
    // scene-wide info
    glUniform1i(glGetUniformLocation(m_shader, "numLights"), numLights);
    glUniform1f(glGetUniformLocation(m_shader, "k_a"), k_a);
    glUniform1f(glGetUniformLocation(m_shader, "k_d"), k_d);
    glUniform1f(glGetUniformLocation(m_shader, "k_s"), k_s);

    GLuint modelLocation = glGetUniformLocation(m_shader, "model");
    for (Shape *shape : shapes) {
        // bind Sphere Vertex Data
        glBindVertexArray(*(shape->getVAO()));

        // pass in m_model as a uniform into the shader program
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &(shape->getCTM()[0][0]));
        SceneMaterial mat = shape->getMat();
        glUniform3f(glGetUniformLocation(m_shader, "cAmbient"), mat.cAmbient[0], mat.cAmbient[1], mat.cAmbient[2]);
        glUniform3f(glGetUniformLocation(m_shader, "cDiffuse"), mat.cDiffuse[0], mat.cDiffuse[1], mat.cDiffuse[2]);
        glUniform3f(glGetUniformLocation(m_shader, "cSpecular"), mat.cSpecular[0], mat.cSpecular[1], mat.cSpecular[2]);
        glUniform1f(glGetUniformLocation(m_shader, "shininess"), mat.shininess);

        // draw Command
        glDrawArrays(GL_TRIANGLES, 0, shape->generateShape().size() / 6);
    }
    // Bind the default framebuffer, clear color/depth buffers
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind our texture shader
    glUseProgram(m_texture_shader);

    // set your bool uniform on whether or not to filter the texture drawn
    glUniform1f(glGetUniformLocation(m_texture_shader, "isInverting"), settings.perPixelFilter);
    glUniform1f(glGetUniformLocation(m_texture_shader, "isSharpening"), settings.kernelBasedFilter);
    glUniform1f(glGetUniformLocation(m_texture_shader, "pixelW"), 1.f / m_fbo_width);
    glUniform1f(glGetUniformLocation(m_texture_shader, "pixelH"), 1.f / m_fbo_height);
    glBindVertexArray(m_fullscreen_vao);

    // bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);

    // unbind Vertex Array
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // students: anything requiring OpenGL calls when the program starts should be done here
    camera.updateWidthHeight(w, h);
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;
    makeFBO();
}

/* PROCEDURAL GENERATION START */
void Realtime::generateCity(WFCGrid &grid) {
    const float tileSize = 5.f;

    std::cout << "grid height: " << 20 << std::endl;
    std::cout << "grid width: " << 20 << std::endl;

    const int protectedXStart = -3;
    const int protectedXEnd = -1;
    const int protectedYStart = -2;
    const int protectedYEnd = 0;

    Mesh windowMesh = Mesh(glm::mat4(1), SceneMaterial(), "scenefiles/nighthawks/building_window.obj");
    Mesh windowMesh1 = Mesh(glm::mat4(1), SceneMaterial(), "scenefiles/nighthawks/building_window1.obj");
    Mesh windowMesh2 = Mesh(glm::mat4(1), SceneMaterial(), "scenefiles/nighthawks/building_window2.obj");
    Mesh windowMesh3 = Mesh(glm::mat4(1), SceneMaterial(), "scenefiles/nighthawks/building_window3.obj");
    std::vector<float> meshes[4];
    meshes[0] = windowMesh.generateShape();
    meshes[1] = windowMesh1.generateShape();
    meshes[2] = windowMesh2.generateShape();
    meshes[3] = windowMesh3.generateShape();

    for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++) {

            // Skip tiles within the protected area
            if (x - 10 >= protectedXStart && x - 10 < protectedXEnd &&
                y - 10 >= protectedYStart && y - 10 < protectedYEnd) {
                continue;
            }

            Tile& tile = grid.grid[y][x];

            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(x * tileSize, 0.f, y * tileSize));

            SceneMaterial mat;
            mat.cDiffuse = glm::vec4(0.8f, 0.4f, 0.2f, 1.f);
            mat.cAmbient = glm::vec4(0.2f, 0.1f, 0.05f, 1.f);
            mat.cSpecular = glm::vec4(1.0f);
            mat.shininess = 16.0f;
            SceneMaterial roofMat;
            roofMat = mat;
            roofMat.cDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
            roofMat.cAmbient = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
            roofMat.cSpecular = glm::vec4(1.0f);
            roofMat.shininess = 5.f;

            glm::mat4 roofMatrix = glm::translate(modelMatrix, glm::vec3(0.f, 1.f, 0.f));
            SceneMaterial doorMat;
            glm::mat4 doorMatrix;
            SceneMaterial bottomMat;
            SceneMaterial topMat;
            shapes.push_back(new Building(modelMatrix, mat, 7, 12, meshes));
            /**
            switch (tile.type) {
            case SMALL_BUILDING: {
                mat.cDiffuse = glm::vec4(0.8, 0.6, 0.5, 1.0); // Brick color
                float height = 2.5f + rand() % 2;
                modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, height / 2.f, 0.f)); // Adjust for bottom alignment
                modelMatrix = glm::scale(modelMatrix, glm::vec3(5.f, height, 5.f));
                shapes.push_back(new Cube(modelMatrix, mat));
                break;
            }
            case MEDIUM_BUILDING: {
                mat.cDiffuse = glm::vec4(0.4, 0.4, 0.6, 1.0); // Concrete color
                float height = 4.f + rand() % 2;
                modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, height / 2.f, 0.f)); // Adjust for bottom alignment
                modelMatrix = glm::scale(modelMatrix, glm::vec3(5.f, height, 5.f));
                shapes.push_back(new Cube(modelMatrix, mat));
                break;
            }
            case TALL_BUILDING: {
                float totalHeight = 8.f + rand() % 3; // Total building height
                float bottomHeight = totalHeight / 2.f; // Height of the bottom layer
                float topHeight = totalHeight - bottomHeight; // Height of the top layer

                // Bottom layer material (teal)
                bottomMat = mat;
                bottomMat.cDiffuse = glm::vec4(0.0, 0.5, 0.5, 1.0); // Teal color

                // Top layer material (red)
                topMat = mat;
                topMat.cDiffuse = glm::vec4(0.8, 0.1, 0.1, 1.0); // Red color

                // Bottom layer (with door)
                glm::mat4 bottomMatrix = glm::translate(modelMatrix, glm::vec3(0.f, bottomHeight / 2.f, 0.f));
                glm::mat4 bottomScaledMatrix = glm::scale(bottomMatrix, glm::vec3(5.f, bottomHeight, 5.f));
                shapes.push_back(new Cube(bottomScaledMatrix, bottomMat));

                // Top layer (with windows)
                glm::mat4 topMatrix = glm::translate(modelMatrix, glm::vec3(0.f, bottomHeight + topHeight / 2.f, 0.f));
                glm::mat4 topScaledMatrix = glm::scale(topMatrix, glm::vec3(5.f, topHeight, 5.f));
                shapes.push_back(new Cube(topScaledMatrix, topMat));

                // Optional: Add details like windows or doors by adding more shapes here

                // Add antennae on top layer
                roofMat = mat;
                roofMat.cDiffuse = glm::vec4(0.7, 0.7, 0.7, 1.0); // Metal

                // Use topMatrix to position antennae on top
                glm::mat4 antennaBaseMatrix = glm::translate(topMatrix, glm::vec3(0.f, topHeight / 2.f, 0.f));
                glm::mat4 antennaMatrix1 = glm::translate(antennaBaseMatrix, glm::vec3(-0.3f, 1.f, -0.3f));
                antennaMatrix1 = glm::scale(antennaMatrix1, glm::vec3(0.25f, 2.f, 0.25f));
                shapes.push_back(new Cylinder(antennaMatrix1, roofMat));

                glm::mat4 antennaMatrix2 = glm::translate(antennaBaseMatrix, glm::vec3(0.4f, 1.f, 0.4f));
                antennaMatrix2 = glm::scale(antennaMatrix2, glm::vec3(0.25f, 1.5f, 0.25f));
                shapes.push_back(new Cylinder(antennaMatrix2, roofMat));

                break;
            }
            default:
                break;
            }**/
        }
    }



}

/* PROCEDURAL GENERATION END */

/**
 * @brief Realtime::sceneChanged called whenever a new scene is loaded (so only once in nighthawks)
 */
void Realtime::sceneChanged() {
    RenderData data;

    // clean up old shape array
    SceneParser::parse(settings.sceneFilePath, data);
    RealtimeUtils::clearArrays(shapes, this);
    for (Shape *shape : shapes) {
        delete shape;
    }
    shapes.clear();
    shapes.resize(data.shapes.size());

    // setup camera and bezier curve
    camera = Camera(data.cameraData, size().width(), size().height(), settings.nearPlane, settings.farPlane);
    bezier = Bezier();

    /* PROCEDURAL GENERATION START */
    WFCGrid grid(1, 1); // 10 x 10 grid
    while (!grid.isFullyCollapsed()) {
        grid.collapse();
    }

    generateCity(grid);
    /* PROCEDURAL GENERATION END */

    // process shape data
    for (int i = 0; i < data.shapes.size(); i++) {
        RenderShapeData shape = data.shapes[i];

        switch (shape.primitive.type) {
            case PrimitiveType::PRIMITIVE_CUBE:
                shapes[i] = new Cube(shape.ctm, shape.primitive.material);
                break;
            case PrimitiveType::PRIMITIVE_CONE:
                shapes[i] = new Cone(shape.ctm, shape.primitive.material);
                break;
            case PrimitiveType::PRIMITIVE_CYLINDER:
                shapes[i] = new Cylinder(shape.ctm, shape.primitive.material);
                break;
            case PrimitiveType::PRIMITIVE_SPHERE:
                shapes[i] = new Sphere(shape.ctm, shape.primitive.material);
                break;
            case PrimitiveType::PRIMITIVE_MESH:
                shapes[i] = new Mesh(shape.ctm, shape.primitive.material, shape.primitive.meshfile);
                break;
        }
    }

    // process light data
    numLights = fmin(data.lights.size(), 8);
    for (int i = 0; i < numLights; i++) {
        lights[i] = data.lights[i];
    }

    // set global constants
    k_a = data.globalData.ka;
    k_s = data.globalData.ks;
    k_d = data.globalData.kd;

    RealtimeUtils::buildArrays(shapes, this, settings.shapeParameter1, settings.shapeParameter2);
    update(); // asks for a PaintGL() call to occur
}

/**
 * @brief Realtime::settingsChanged called when settings change. should happen minimally in nighthawks
 */
void Realtime::settingsChanged() {
    // updates camera and shape arrays accordingly
    camera.updateClippingPlanes(settings.nearPlane, settings.farPlane);
    RealtimeUtils::clearArrays(shapes, this);
    RealtimeUtils::buildArrays(shapes, this, settings.shapeParameter1, settings.shapeParameter2);
    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        glm::mat4 ctm = camera.getViewMatrixInverted();
        glm::vec3 rightVector = glm::normalize(glm::vec3(ctm[0][0], ctm[0][1], ctm[0][2]));
        camera.rotate(glm::vec3(0, 1, 0), deltaX * 0.01);
        camera.rotate(rightVector, deltaY * 0.01);

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    glm::mat4 ctm = camera.getViewMatrixInverted();

    // get forward and right vectors
    float delta = 5.f * deltaTime;
    glm::vec3 lookVector = -delta * glm::normalize(glm::vec3(ctm[2][0], ctm[2][1], ctm[2][2]));
    glm::vec3 rightVector = delta * glm::normalize(glm::vec3(ctm[0][0], ctm[0][1], ctm[0][2]));

    // translate based on keys pressed
    if (m_keyMap[Qt::Key_W]) camera.translate(lookVector);
    if (m_keyMap[Qt::Key_S]) camera.translate(-lookVector);
    if (m_keyMap[Qt::Key_D]) camera.translate(rightVector);
    if (m_keyMap[Qt::Key_A]) camera.translate(-rightVector);
    if (m_keyMap[Qt::Key_Space]) camera.translate(glm::vec3(0, delta, 0));
    if (m_keyMap[Qt::Key_Control]) camera.translate(glm::vec3(0, -delta, 0));

    glm::vec3 bezierTranslate = bezier.incrementTime(0.25f * deltaTime);
    camera.translate(bezierTranslate);

    // update the look vector to keep the camera looking at the building
    glm::vec3 buildingPosition(0.8f, 0.8f, -0.2f); // building's focus position
    glm::vec3 cameraPosition = glm::vec3(camera.getCamPos()); // current camera position
    glm::vec3 look = glm::normalize(buildingPosition - cameraPosition);
    camera.setLookVector(look); // update look vector

    update(); // asks for a PaintGL() call to occur
}

void Realtime::makeFBO(){
    this->makeCurrent();
    // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);
    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
