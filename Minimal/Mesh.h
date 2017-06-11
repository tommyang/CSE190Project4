#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/types.h>


struct Vertex {
    // Position
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // TexCoords
    glm::vec2 TexCoords;
};

struct Texture {
    GLuint id;
    string type;
    aiString path;
};

struct Material {
    aiColor3D ambient;
    aiColor3D diffuse;
    aiColor3D specular;
    float shininess;
};

class Mesh {
public:
    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    Material material;

    glm::mat4 toWorld;
    GLuint uProjection, uModel, uView, texture_ID, uAmbient, uDiffuse, uSpecular, uShininess;

    /*  Functions  */
    // Constructor
    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures, Material material)
    {
        toWorld = glm::mat4(1.0f);

        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->material = material;

        // Now that we have all the required data, set the vertex buffers and its attribute pointers.
        this->setupMesh();
    }


	void draw(glm::mat4 C, GLint shaderProgram, glm::mat4 P, glm::mat4 V)
	{
		uProjection = glGetUniformLocation(shaderProgram, "projection");
		uModel = glGetUniformLocation(shaderProgram, "model");
		uView = glGetUniformLocation(shaderProgram, "view");
		uAmbient = glGetUniformLocation(shaderProgram, "material.ambient");
		uDiffuse = glGetUniformLocation(shaderProgram, "material.diffuse");
		uSpecular = glGetUniformLocation(shaderProgram, "material.specular");
		uShininess = glGetUniformLocation(shaderProgram, "material.shininess");
		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(uModel, 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(uView, 1, GL_FALSE, &C[0][0]);
		glUniform3f(uAmbient, material.ambient.r, material.ambient.g, material.ambient.b);
		glUniform3f(uDiffuse, material.diffuse.r, material.diffuse.g, material.diffuse.b);
		glUniform3f(uSpecular, material.specular.r, material.specular.g, material.specular.b);
		glUniform1f(uShininess, material.shininess);


		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture_ID);
		//glUniform1i(glGetUniformLocation(shaderProgram, (opt == 0 ? "left" : "right")), 0);


		// Bind appropriate textures
		///*
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;
		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
											  // Retrieve texture number (the N in diffuse_textureN)
			stringstream ss;
			string number;
			string name = this->textures[i].type;
			if (name == "texture_diffuse")
				ss << diffuseNr++; // Transfer GLuint to stream
			else if (name == "texture_specular")
				ss << specularNr++; // Transfer GLuint to stream
			number = ss.str();
			// Now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shaderProgram, (name + number).c_str()), i);
			// And finally bind the texture
			glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
		}
		//*/

		// Also set each mesh's shininess property to a default value (if you want you could extend this to another mesh property and possibly change this value)
		// glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 16.0f);

		// Draw mesh
		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Always good practice to set everything back to defaults once configured.
		/*
		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		*/
	}

    void update() {

    }

private:
    /*  Render data  */
    GLuint VAO, VBO, EBO;

    /*  Functions    */
    // Initializes all the buffer objects/arrays
    void setupMesh()
    {
        // Create buffers/arrays
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);
        // Load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Set the vertex attribute pointers
        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
        // Vertex Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
        // Vertex Texture Coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};
