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
#include "utils/shaderloader.h"
#include "utils/realtimeutils.h"

// ================== Project 5: Lights, Camera

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

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    RealtimeUtils::clearArrays(shapes, this);

    for (Shape *shape : shapes) {
        delete shape;
    }

    glDeleteProgram(m_shader);
    glDeleteProgram(m_texture_shader);
    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();
    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, m_screen_width, m_screen_height);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

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

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();
}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate the shader program
    glUseProgram(m_shader);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // camera info
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"), 1, GL_FALSE, &(camera.getViewMatrix())[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "proj"), 1, GL_FALSE, &(camera.getProjMatrix())[0][0]);
    glUniform4fv(glGetUniformLocation(m_shader, "camPos"), 1, &(camera.getCamPos())[0]);

    // send light info
    for (int i = 0; i < numLights; i++) {
        SceneLightData light = lights[i];
        std::string baseName = "lights[" + std::to_string(i) + "]";
        GLuint typeLoc = glGetUniformLocation(m_shader, (baseName + ".type").c_str());
        GLuint colorLoc = glGetUniformLocation(m_shader, (baseName + ".color").c_str());
        GLuint dirLoc = glGetUniformLocation(m_shader, (baseName + ".dir").c_str());
        GLuint functionLoc = glGetUniformLocation(m_shader, (baseName + ".function").c_str());
        GLuint posLoc = glGetUniformLocation(m_shader, (baseName + ".pos").c_str());

        glUniform3f(colorLoc, light.color[0], light.color[1], light.color[2]);
        glUniform3f(dirLoc, light.dir[0], light.dir[1], light.dir[2]);
        glUniform3f(functionLoc, light.function[0], light.function[1], light.function[2]);
        glUniform3f(posLoc, light.pos[0], light.pos[1], light.pos[2]);

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
    glUniform1i(glGetUniformLocation(m_shader, "numLights"), numLights);
    glUniform1f(glGetUniformLocation(m_shader, "k_a"), k_a);
    glUniform1f(glGetUniformLocation(m_shader, "k_d"), k_d);
    glUniform1f(glGetUniformLocation(m_shader, "k_s"), k_s);

    GLuint modelLocation = glGetUniformLocation(m_shader, "model");
    for (Shape *shape : shapes) {
        // Bind Sphere Vertex Data
        glBindVertexArray(*(shape->getVAO()));

        // pass in m_model as a uniform into the shader program
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &(shape->getCTM()[0][0]));
        SceneMaterial mat = shape->getMat();
        glUniform3f(glGetUniformLocation(m_shader, "cAmbient"), mat.cAmbient[0], mat.cAmbient[1], mat.cAmbient[2]);
        glUniform3f(glGetUniformLocation(m_shader, "cDiffuse"), mat.cDiffuse[0], mat.cDiffuse[1], mat.cDiffuse[2]);
        glUniform3f(glGetUniformLocation(m_shader, "cSpecular"), mat.cSpecular[0], mat.cSpecular[1], mat.cSpecular[2]);
        glUniform1f(glGetUniformLocation(m_shader, "shininess"), mat.shininess);

        // Draw Command
        glDrawArrays(GL_TRIANGLES, 0, shape->generateShape().size() / 6);
    }


    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_texture_shader);
    // Set your bool uniform on whether or not to filter the texture drawn
    glUniform1f(glGetUniformLocation(m_texture_shader, "isInverting"), settings.perPixelFilter);
    glUniform1f(glGetUniformLocation(m_texture_shader, "isSharpening"), settings.kernelBasedFilter);
    glUniform1f(glGetUniformLocation(m_texture_shader, "pixelW"), 1.f / m_fbo_width);
    glUniform1f(glGetUniformLocation(m_texture_shader, "pixelH"), 1.f / m_fbo_height);
    glBindVertexArray(m_fullscreen_vao);
    // Bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Vertex Array
    glBindVertexArray(0);
    glUseProgram(0);


}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    camera.updateWidthHeight(w, h);
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;
    makeFBO();
}

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

    camera = Camera(data.cameraData, size().width(), size().height(), settings.nearPlane, settings.farPlane);
    bezier = Bezier();
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

    glm::vec3 bezierTranslate = bezier.incrementTime(deltaTime);
    camera.translate(bezierTranslate);

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
