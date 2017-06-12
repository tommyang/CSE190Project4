//
//  Sphere.cpp
//  CSE167Proj3
//
//  Created by Tommy Yang on 11/2/16.
//  Copyright © 2016 tOMG. All rights reserved.
//

#include "Sphere.h"
#include <iostream>
#include <fstream>

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
			uvs.push_back(glm::vec2(U, V));
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
	glGenBuffers(1, &uv_ID);
    
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

	glBindBuffer(GL_ARRAY_BUFFER, uv_ID);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		2, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		2 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

					 // Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);

	this->loadCubemap();
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glUniform1i(glGetUniformLocation(shaderProgram, "myTextureSampler"), 0);


    // Now draw the cube. We simply need to bind the VAO associated with it.
    glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, uv_ID);

    // Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
    glDrawElements(GL_TRIANGLES,  indices.size(), GL_UNSIGNED_INT, 0);

    // Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
    glBindVertexArray(0);
}

void Sphere::update() {
	velocity.y -= 0.0001f;
	toWorld = glm::translate(glm::mat4(1.0f), velocity) * toWorld;
	glm::vec3 pos = toWorld * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	if (pos.x > 1.0f || pos.x < -1.0f) {
		velocity.x *= (-1.0f);
		toWorld = glm::translate(glm::mat4(1.0f), velocity) * toWorld;
		velocity = velocity * 0.9f;
	}
	if (pos.y > 3.0f || pos.y < 0.18f) {
		velocity.y *= (-1.0f);
		toWorld = glm::translate(glm::mat4(1.0f), velocity) * toWorld;
		velocity = velocity * 0.9f;
	}
	if (pos.z > 0.5f || pos.z < -0.5f) {
		velocity.z *= (-1.0f);
		toWorld = glm::translate(glm::mat4(1.0f), velocity) * toWorld;
		velocity = velocity * 0.9f;
	}
}

// Load textures for skybox
GLuint Sphere::loadCubemap() {

	glGenTextures(1, &texture_ID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_2D, texture_ID);

	// Load front
	image = loadPPM("C:/Users/degu/Desktop/CSE190Project4/Minimal/vr_test_pattern.ppm", width, height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Sets texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	free(image);
	// Unbinds texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture_ID;
}

#pragma warning(disable : 4996)  
unsigned char* Sphere::loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return 0;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;

		return 0;
	}

	return rawData;
}
