#include "mesh.h"
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <cmath>

Mesh::Mesh(glm::mat4 in_ctm, SceneMaterial in_mat, std::string meshFile) : Shape(in_ctm, in_mat) {
    std::vector<glm::vec3> faces;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> normal_indices;
    m_vertexData = std::vector<float>();
    parseFile(meshFile, faces, vertices, normals, normal_indices);
    populateTriangles(faces, vertices, normals, normal_indices);
}

/**
 * Parses the mesh file and stores
 * vertex and face info in vectors
 */
void Mesh::parseFile(std::string meshFile, std::vector<glm::vec3>& faces, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& normal_indices) {
    QFile inputFile(QString::fromStdString(meshFile));
    std::cout << "mesh file: " << meshFile << std::endl;
    if (inputFile.open(QIODevice::ReadOnly))
    {
        std::cout << "opened" << std::endl;
        QTextStream in(&inputFile);
        glm::vec3 face;
        glm::vec3 normal_index;
        QStringList lineSplitSplit;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList lineSplit = line.split(" ");

            if (lineSplit.size() >= 4) {
                QString first = lineSplit[0];
                if (first == "v") {
                    glm::vec3 vertex(lineSplit[1].toFloat(), lineSplit[2].toFloat(), lineSplit[3].toFloat());
                    vertices.push_back(vertex);
                }
                else if (first == "vn") {
                    glm::vec3 normal(lineSplit[1].toFloat(), lineSplit[2].toFloat(), lineSplit[3].toFloat());
                    normals.push_back(normal);
                }
                else if (first == "f") {
                    for (int i = 0; i < 3; i++) {
                        lineSplitSplit = lineSplit[i + 1].split("/");
                        face[i] = lineSplitSplit[0].toFloat();
                        if (lineSplitSplit.size() >= 3) {
                            normal_index[i] = lineSplitSplit[2].toFloat();
                        }
                        else {
                            normal_index[i] = NAN;
                        }
                    }
                    faces.push_back(face);
                    normal_indices.push_back(normal_index);
                    // if quad
                    if (lineSplit.size() == 5) {
                        faces.push_back(glm::vec3(face[2], lineSplit[4].split("/")[0].toFloat(), face[0]));
                        lineSplitSplit = lineSplit[4].split("/");
                        if (lineSplitSplit.size() >= 3) {
                            normal_indices.push_back(glm::vec3(normal_index[2], lineSplit[4].split("/")[2].toFloat(), normal_index[0]));
                        }
                        else {
                            normal_indices.push_back(glm::vec3(normal_index[2], NAN, normal_index[0]));
                        }
                    }
                }
            }
        }
        inputFile.close();
    }
}

void Mesh::populateTriangles(std::vector<glm::vec3>& faces, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& normal_indices) {
    for (int i = 0; i < faces.size(); i++) {
        glm::vec3 face = faces[i];

        glm::vec3 normal_index = normal_indices[i];

        glm::vec3 e1 = vertices[face[1] - 1] - vertices[face[0] - 1];
        glm::vec3 e2 = vertices[face[2] - 1] - vertices[face[0] - 1];
        glm::vec3 face_n = glm::normalize(glm::cross(e1, e2));
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = vertices[face[j] - 1];
            insertVec3(m_vertexData, v);
            if (std::isnan(normal_index[j])) {
                insertVec3(m_vertexData, face_n);
            }
            else {
                glm::vec3 n = normals[normal_index[j] - 1];
                insertVec3(m_vertexData, n);
            }
        }
    }
}

void Mesh::updateParams(int param1, int param2) {}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Mesh::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
