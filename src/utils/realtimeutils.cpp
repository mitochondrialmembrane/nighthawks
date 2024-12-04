#include "realtimeutils.h"
#include <GL/glew.h>

namespace RealtimeUtils {

/**
 * Deletes the vbo and vao of each shape
 */
void clearArrays(std::vector<Shape*> shapes, QOpenGLWidget *widget) {
    widget->makeCurrent();

    for (Shape* shape : shapes) {
        // Delete the VBO
        GLuint* vbo = shape->getVBO();
        if (vbo && *vbo != 0) {
            glDeleteBuffers(1, vbo);
            *vbo = 0;
        }

        // Delete the VAO
        GLuint* vao = shape->getVAO();
        if (vao && *vao != 0) {
            glDeleteVertexArrays(1, vao);
            *vao = 0;
        }
    }
}

/**
 * Creates a vao and vbo for each shape depending on shape parameters
 */
void buildArrays(std::vector<Shape*> shapes, QOpenGLWidget *widget, int shapeParameter1, int shapeParameter2) {
    widget->makeCurrent();
    for (Shape* shape : shapes) {
        GLuint *vbo = shape->getVBO();
        GLuint *vao = shape->getVAO();

        // Generate and bind vbo
        glGenBuffers(1, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, *vbo);
        // Generate shape data
        shape->updateParams(shapeParameter1, shapeParameter2);
        std::vector<float> buffer = shape->generateShape();

        // Send data to VBO
        glBufferData(GL_ARRAY_BUFFER,buffer.size() * sizeof(GLfloat), buffer.data(), GL_STATIC_DRAW);
        // Generate, and bind vao
        glGenVertexArrays(1, vao);
        glBindVertexArray(*vao);
        // Enable and define attribute 0 to store vertex positions and 1 to store vertex normals
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

}
