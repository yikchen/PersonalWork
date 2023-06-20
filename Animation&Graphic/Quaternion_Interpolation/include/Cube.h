#pragma once

#include <vector>
#include "Quaternion.h"

class Cube {
private:
    GLuint VAO;
    GLuint VBO_positions, VBO_normals, EBO;
    

    glm::mat4 model;
   

    // Cube Information
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

public:
    Quaternion* q;
    float x;
    float y;
    float z;

    glm::vec3 color;
    Cube(glm::vec3 cubeMin = glm::vec3(-0.25, -0.75, -0.5), glm::vec3 cubeMax = glm::vec3(0.25, 0.75, 0.5));
    ~Cube();


    void drawFrame(const glm::mat4& viewProjMtx, GLuint shader);
    void draw(const glm::mat4& viewProjMtx, GLuint shader);
    void update();
    void update(float degreeX, float degreeY, float degreeZ);
    void update(float t,Quaternion& q0, Quaternion& q1);
    void update(float t, Quaternion& q0, Quaternion& q1, Quaternion& q2, Quaternion& q3);

    void spin(float deg);
};