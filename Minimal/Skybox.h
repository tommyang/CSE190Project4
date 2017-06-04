/****************************/
/*****	by ANDY THAI	*****/
/****************************/

#ifndef _SKYBOX_H_
#define _SKYBOX_H_

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
#include <glm/gtc/matrix_transform.hpp>

class Skybox
{
public:
	Skybox();
	Skybox(float mult);
	~Skybox();

	glm::mat4 toWorld;

	void initialize();
	void draw(GLuint, glm::mat4, glm::mat4);
	void sendLight(GLuint shaderProgram);
	unsigned char* loadPPM(const char*, int&, int&);

	// Cubemap
	void loadCubemap();
	void useCubemap(int eyeIdx);
	glm::vec3 direction = glm::vec3(-0.0459845f, 0.0925645f, 0.994644f);

	// These variables are needed for the shader program
	GLuint VBO, VAO, uv_ID;
	GLuint uProjection, uModel, uView, texture_ID_left, texture_ID_right, texture_ID_self, curTextureID;

	/*
	GLfloat vertices[8][3] = {
		// "Front" vertices
		{ -2.0, -2.0,  2.0 },{ 2.0, -2.0,  2.0 },{ 2.0,  2.0,  2.0 },{ -2.0,  2.0,  2.0 },
		// "Back" vertices
		{ -2.0, -2.0, -2.0 },{ 2.0, -2.0, -2.0 },{ 2.0,  2.0, -2.0 },{ -2.0,  2.0, -2.0 }
	};
	
	GLfloat normals[12][3] = {
		{ 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 },{ 0.0, -1.0, 0.0 },{ 0.0, -1.0, 0.0 },
		{ 0.0, 0.0, -1.0 },{ 0.0, 0.0, -1.0 },{ 0.0, 1.0, 0.0 },{ 0.0, 1.0, 0.0 },
		{ 1.0, 0.0, 0.0 },{ 1.0, 0.0, 0.0 },{ -1.0, 0.0, 0.0 },{ -1.0, 0.0, 0.0 }
	};
	*/
	GLfloat vertices[108] = {
		-20.0, -20.0,  20.0, // triangle 1 : begin
		20.0, -20.0,  20.0,
		20.0,  20.0,  20.0, // triangle 1 : end
		20.0,  20.0,  20.0, // triangle 2 : begin
		-20.0,  20.0,  20.0,
		-20.0, -20.0,  20.0, // triangle 2 : end
		//
		20.0, -20.0,  20.0,
		20.0, -20.0, -20.0,
		20.0,  20.0, -20.0,
		20.0,  20.0, -20.0,
		20.0,  20.0,  20.0,
		20.0, -20.0,  20.0,
		//
		-20.0,  20.0, -20.0,
		20.0,  20.0, -20.0,
		20.0, -20.0, -20.0,
		20.0, -20.0, -20.0,
		-20.0, -20.0, -20.0,
		-20.0,  20.0, -20.0,
		//
		-20.0, -20.0, -20.0,
		-20.0, -20.0,  20.0,
		-20.0,  20.0,  20.0,
		-20.0,  20.0,  20.0,
		-20.0,  20.0, -20.0,
		-20.0, -20.0, -20.0,
		//
		-20.0, -20.0, -20.0,
		20.0, -20.0, -20.0,
		20.0, -20.0,  20.0,
		20.0, -20.0,  20.0,
		-20.0, -20.0,  20.0,
		-20.0, -20.0, -20.0,
		//
		-20.0,  20.0,  20.0,
		20.0,  20.0,  20.0,
		20.0,  20.0, -20.0,
		20.0,  20.0, -20.0,
		-20.0,  20.0, -20.0,
		-20.0,  20.0,  20.0
	};

	const GLfloat uvs[72] = {
		// front
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		// top
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		// back
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		// bottom
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		// left
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		// right
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};
};

// Note that GL_QUADS is deprecated in modern OpenGL (and removed from OSX systems).
// This is why we need to draw each face as 2 triangles instead of 1 quadrilateral
/*
const GLuint skybox_indices[6][6] = {
	// Front face
	{ 0, 1, 2, 2, 3, 0 },
	// Top face
	{ 1, 5, 6, 6, 2, 1 },
	// Back face
	{ 7, 6, 5, 5, 4, 7 },
	// Bottom face
	{ 4, 0, 3, 3, 7, 4 },
	// Left face
	{ 4, 5, 1, 1, 0, 4 },
	// Right face
	{ 3, 2, 6, 6, 7, 3 }
};
*/

#endif