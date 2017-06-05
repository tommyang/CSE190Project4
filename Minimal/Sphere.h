//
//  Sphere.hpp
//  CSE167Proj3
//
//  Created by Tommy Yang on 11/2/16.
//  Copyright © 2016 tOMG. All rights reserved.
//

#ifndef Sphere_hpp
#define Sphere_hpp

#include <stdio.h>
#include <vector>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Sphere{
public:
    double radius = 2;
    int slices = 15;
    int stacks = 15;
    
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> vertices;
    
    Sphere();
    ~Sphere();
    
    glm::mat4 toWorld;
    
    GLuint VBO, VAO, EBO;
    GLuint uProjection, uModel, uView;
    
    void draw(GLuint, glm::mat4 P, glm::mat4 V);
    void update();
};

#endif /* Sphere_hpp */
