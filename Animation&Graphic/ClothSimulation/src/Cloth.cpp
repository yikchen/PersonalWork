#include "Cloth.h"
#include <iostream>
Cloth::Cloth(float size, float wps, int x, int y, float stiff, float d_stiff) { //size: size x size == meter x meter, wps: weight per meter^2, x,y: number of particle in x,y direction
	row = x;
	col = y;
	stiffness = stiff;
	d_stiffness = d_stiff;

	
	model = glm::mat4(1.0f);
	color = glm::vec3(1.0f, 0.95f, 0.1f);

	float mass = size * wps;
	float mpp = mass / (x * y);
	float distance = size / y;
	
	/**
	* Creating partciles to serve as vertices of the cloth
	*/
	for (int i = 0; i < row; i++) {
		glm::vec3 pos(-1.0, 2.0 ,1.0);	
		pos -= (float)i * glm::vec3(0, 0, distance);
		for (int j = 0; j < col; j++) {
			Particle* toAdd = new Particle(pos, mpp);
			if (i == 0 ) {
				toAdd->setfix();
			}
			/*if (i == 0 && j == col-1 || i ==0 && j ==0) {
				toAdd->setfix();
			}*/
			vertices.push_back(toAdd);
			position.push_back(pos);
			pos += glm::vec3(distance, 0, 0);
		}
	}

	/**
	* Creating SpringDamper to connect each particle in the cloth
	*/
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			if (i == 0 && j < col-1) {//handling particle in the top row
				SpringDamper* newSpring = new SpringDamper(vertices[i * col + j], vertices[i * col + j + 1], stiffness, d_stiffness, distance);
				springs.push_back(newSpring);
			}
			else if (i > 0 && j ==0) { //handling particles that on the the left edge
				float diagonal = glm::sqrt(2 * glm::pow(distance, 2));
				SpringDamper* newSp1 = new SpringDamper(vertices[i * col + j], vertices[(i - 1) * col + j], stiffness, d_stiffness, distance);
				SpringDamper* newSp2 = new SpringDamper(vertices[i * col + j], vertices[i * col + j + 1], stiffness, d_stiffness, distance);
				SpringDamper* newSp3 = new SpringDamper(vertices[i * col + j], vertices[(i - 1) * col + j + 1], stiffness, d_stiffness, diagonal);
				springs.push_back(newSp1);
				springs.push_back(newSp2);
				springs.push_back(newSp3);
			}
			else if (i > 0 && j > 0 && j < col - 1) { //handling particles that are in the middle
				float diagonal = glm::sqrt(2 * glm::pow(distance, 2));
				SpringDamper* newSp1 = new SpringDamper(vertices[i * col + j], vertices[(i - 1) * col + j], stiffness, d_stiffness, distance);
				SpringDamper* newSp2 = new SpringDamper(vertices[i * col + j], vertices[i * col + j + 1], stiffness, d_stiffness, distance);
				SpringDamper* newSp3 = new SpringDamper(vertices[i * col + j], vertices[(i - 1) * col + j + 1], stiffness, d_stiffness, diagonal);
				SpringDamper* newSp4 = new SpringDamper(vertices[i * col + j], vertices[(i - 1) * col + j - 1], stiffness, d_stiffness, diagonal);
				springs.push_back(newSp1);
				springs.push_back(newSp2);
				springs.push_back(newSp3);
				springs.push_back(newSp4);
			}
			else if (i > 0 && j == col - 1) { // handling particles that are on the right edge
				float diagonal = glm::sqrt(2 * glm::pow(distance, 2));
				SpringDamper* newSp1 = new SpringDamper(vertices[i * col + j], vertices[(i - 1) * col + j], stiffness, d_stiffness, distance);
				SpringDamper* newSp2 = new SpringDamper(vertices[i * col + j], vertices[(i - 1) * col + j - 1], stiffness, d_stiffness, diagonal);
				springs.push_back(newSp1);
				springs.push_back(newSp2);
			}

		}
	}

	/**
	* Creating Triangle to render and to apply aerodynamic force
	*/
	for (int i = 0; i + 1 < row; i++) {
		for (int j = 0; j + 1 < col; j++) {
			int idx0 = i * col + j;
			int idx1 = i * col + j + 1;
			int idx2 = (i + 1) * col + j;
			int idx3 = (i + 1) * col + j + 1;
			Triangle* t1 = new Triangle(vertices[idx0], vertices[idx2], vertices[idx3], 1.225f, 1.28f);
			Triangle* t2 = new Triangle(vertices[idx0], vertices[idx3], vertices[idx1], 1.225f, 1.28f);
			triangles.push_back(t1);
			triangles.push_back(t2);
			indices.push_back(idx0);
			indices.push_back(idx2);
			indices.push_back(idx3);
			indices.push_back(idx0);
			indices.push_back(idx3);
			indices.push_back(idx1);

			normal.push_back(t1->getNorm());
			normal.push_back(t2->getNorm());
		}
	}

	// Generate a vertex array (VAO) and two vertex buffer objects (VBO).
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO_positions);
	glGenBuffers(1, &VBO_normals);

	// Bind to the VAO.
	glBindVertexArray(VAO);
	// Bind to the first VBO - We will use it to store the vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * position.size(), position.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Bind to the second VBO - We will use it to store the normals
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normal.size(), normal.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Generate EBO, bind the EBO to the bound VAO and send the data
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// Unbind the VBOs.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Cloth::Update(float deltaT, glm::vec3 pos, glm::vec3 wind) {
	if (glfwGetTime() > 3) {
		for (int i = 0; i < col; i++) {
			//vertices[i]->releasefix();
		}
	}
	
	for (int i = 0; i < col; i++) {
		vertices[i]->setPos(pos);
	}
	/*vertices[0]->setPos(pos);
	vertices[col-1]->setPos(pos);*/
	for (int i = 0; i < springs.size(); i++) {
		springs[i]->computeForce();
	}
	for (int i = 0; i < triangles.size(); i++) {
		triangles[i]->V_air = wind;
		triangles[i]->computeForce();
	}
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i]->Integrate(deltaT);
	}

	for (int i = 0; i < position.size(); i++) {
		position[i] = vertices[i]->getPos();
 	}
	for (int i = 0; i < normal.size(); i++) {
		normal[i] = triangles[i]->getNorm();
	}

	// Bind to the VAO.
	glBindVertexArray(VAO);
	// Bind to the first VBO - We will use it to store the vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * position.size(), position.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Bind to the second VBO - We will use it to store the normals
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normal.size(), normal.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Unbind the VBOs.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Cloth::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
	
	// actiavte the shader program
	glUseProgram(shader);
	// get the locations and send the uniforms to the shader
	glUniformMatrix4fv(glGetUniformLocation(shader, "viewProj"), 1, false, (float*)&viewProjMtx);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, (float*)&model);
	glUniform3fv(glGetUniformLocation(shader, "DiffuseColor"), 1, &color[0]);

	// Bind the VAO
	glBindVertexArray(VAO);

	// draw the points using triangles, indexed with the EBO
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	// Unbind the VAO and shader program
	glBindVertexArray(0);
	glUseProgram(0);
}

Cloth::~Cloth() {
	for (int i = 0; i < vertices.size(); i++) {
		delete vertices[i];
	}
	for (int i = 0; i < springs.size(); i++) {
		delete springs[i];
	}
	for (int i = 0; i < triangles.size(); i++) {
		delete triangles[i];
	}
}