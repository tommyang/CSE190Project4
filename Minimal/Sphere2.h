#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glew.h>

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Sphere
{
public:
	Sphere();
	~Sphere();

	glm::mat4 toWorld;

	void init();
	void cleanup();
	void draw(GLuint, glm::mat4 P, glm::mat4 V);

	int lats, longs;
	bool isInited;
	GLuint m_vao, m_vboVertex, m_vboIndex;
	GLuint uProjection, uModel, uView;
	int numsToDraw;
};

#endif // SPHERE_H