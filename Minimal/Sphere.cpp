//
//  Sphere.cpp
//  CSE167Proj3
//
//  Created by Tommy Yang on 11/2/16.
//  Copyright © 2016 tOMG. All rights reserved.
//

#include "Sphere.h"

Sphere::Sphere() {
    for (int i = 0; i <= stacks; ++i){
        float V   = i / (float) stacks;
        float phi = V * glm::pi<float>();

        for (int j = 0; j <= slices; ++j){
            float U = j / (float) slices;
            float theta = U * (glm::pi<float>() * 2);
            float x = cosf (theta) * sinf (phi)  * radius;
            float y = cosf (phi)  * radius;
            float z = sinf (theta) * sinf (phi)  * radius;
            vertices.push_back(glm::vec3(x, y, z));
        }
    }
    for (int i = 0; i < slices * stacks + slices; ++i){
        indices.push_back (i);
        indices.push_back (i + slices + 1);
        indices.push_back (i + slices);
        indices.push_back (i + slices + 1);
        indices.push_back (i);
        indices.push_back (i + 1);
    }
    
    toWorld = glm::mat4(1.0f);
    
    // Create array object and buffers. Remember to delete your buffers when the object is destroyed!
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
    // Consider the VAO as a container for all your buffers.
    glBindVertexArray(VAO);
    
    // Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
    // you want to draw, such as vertices, normals, colors, etc.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
    // the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    // Enable the usage of layout location 0 (check the vertex shader to see what this is)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
                          3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
                          GL_FLOAT, // What type these components are
                          GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
                          3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
                          (GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.
    
    // We've sent the vertex data over to OpenGL, but there's still something missing.
    // In what order should it draw those vertices? That's why we'll need a GL_ELEMENT_ARRAY_BUFFER for this.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind the VAO now so we don't accidentally tamper with it.
    // NOTE: You must NEVER unbind the element array buffer associated with a VAO!
    glBindVertexArray(0);
}

Sphere::~Sphere() {
    
}

void Sphere::draw(GLuint shaderProgram, glm::mat4 P, glm::mat4 V) {
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModel = glGetUniformLocation(shaderProgram, "model");
	uView = glGetUniformLocation(shaderProgram, "view");
	// Now send these values to the shader program
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(uModel, 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(uView, 1, GL_FALSE, &toWorld[0][0]);


    // Now draw the cube. We simply need to bind the VAO associated with it.
    glBindVertexArray(VAO);
    // Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
    glDrawElements(GL_TRIANGLES,  indices.size(), GL_UNSIGNED_INT, 0);
    // Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
    glBindVertexArray(0);
}

void Sphere::update() {
    
}
