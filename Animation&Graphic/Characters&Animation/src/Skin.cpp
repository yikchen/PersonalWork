#include "Skin.h"
Skin::Skin() {
	model = glm::mat4(1.0f);
	color = glm::vec3(0.1f, 0.5f, 1.0f);
	skel = NULL;
}

bool Skin::Load(const char* file) {
	Tokenizer token;
	token.Open(file);
	
	//load the file set up position, normal, and weight
	while (1) {
		char temp[256];
		token.GetToken(temp);
		if (strcmp(temp, "positions") == 0) {
			int numVertex = token.GetInt();
			token.SkipLine();
			for (int i = 0; i < numVertex; i++) {
				Vertex* newVec = new Vertex();
				newVec->position.x = token.GetFloat();
				newVec->position.y = token.GetFloat();
				newVec->position.z = token.GetFloat();
				position.push_back(glm::vec3(newVec->position.x, newVec->position.y, newVec->position.z));
				vertex.push_back(newVec);
				token.SkipLine();
			}
		}
		else if (strcmp(temp, "normals") == 0) {
			int numNorm = token.GetInt();
			token.SkipLine();
			for (int i = 0; i < numNorm; i++) {
				vertex[i]->normal.x = token.GetFloat();
				vertex[i]->normal.y = token.GetFloat();
				vertex[i]->normal.z = token.GetFloat();
				normal.push_back(glm::vec3(vertex[i]->normal.x, vertex[i]->normal.y, vertex[i]->normal.z));
				token.SkipLine();
			}
		}
		else if (strcmp(temp, "skinweights") == 0) {
			int numWeights = token.GetInt();
			token.SkipLine();
			for (int i = 0; i < numWeights; i++) {
				int count = token.GetInt();
				for (int j = 0; j < count; j++) {
					int joint = token.GetInt();
					float weight = token.GetFloat();
					vertex[i]->setJointWeight(joint, weight);
				}
				token.SkipLine();
			}
		}
		else if (strcmp(temp, "triangles") == 0) {
			int numTri = token.GetInt();
			token.SkipLine();
			for (int i = 0; i < numTri; i++) {
				Triangle* t = new Triangle();
				unsigned int ver0 = token.GetInt();
				unsigned int ver1 = token.GetInt();
				unsigned int ver2 = token.GetInt();
				indices.push_back(ver0);
				indices.push_back(ver1);
				indices.push_back(ver2);

				t->setv0(vertex[ver0]);
				t->setv1(vertex[ver1]);
				t->setv2(vertex[ver2]);
				Tsuit.push_back(t);
			}
		}
		else if (strcmp(temp, "bindings") == 0) {
			int num = token.GetInt();
			for (int i = 0; i < num; i++) {
				token.FindToken("matrix");
				
				token.SkipLine();
				float a1 = token.GetFloat();
				float a2 = token.GetFloat();
				float a3 = token.GetFloat();
				glm::vec4 a = glm::vec4(a1, a2, a3,0);

				token.SkipLine();
				float b1 = token.GetFloat();
				float b2 = token.GetFloat();
				float b3 = token.GetFloat();
				glm::vec4 b = glm::vec4(b1, b2, b3,0);

				token.SkipLine();
				float c1 = token.GetFloat();
				float c2 = token.GetFloat();
				float c3 = token.GetFloat();
				glm::vec4 c = glm::vec4(c1, c2, c3,0);

				token.SkipLine();
				float d1 = token.GetFloat();
				float d2 = token.GetFloat();
				float d3 = token.GetFloat();
				glm::vec4 d = glm::vec4(d1, d2, d3,1);

				glm::mat4 binding = glm::mat4(a, b, c, d);
				bindMatrix.push_back(binding);
			}
			break;
		}
		else {
			token.SkipLine(); // Unrecognized token
		}
	}
	//Finish
	token.Close();

	std::cout << "good" << std::endl;
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

	return true;
}
void Skin::setSkeleton(Skeleton* s) {
	this->skel = s;
}

void Skin::Update() {
	for (int i = 0; i < bindMatrix.size(); i++) {
		if (skinningMatrix.size() < bindMatrix.size()) {
			skinningMatrix.push_back(skel->GetWorldMatrix(i) * glm::inverse(bindMatrix[i]));
		}
		else {
			skinningMatrix[i] = skel->GetWorldMatrix(i) * glm::inverse(bindMatrix[i]);
		}
	}
	for (int i = 0; i < vertex.size(); i++) {
		glm::vec3 newPos = glm::vec3(0, 0, 0);
		glm::vec3 newNorm = glm::vec3(0, 0, 0);
		for (int j = 0; j < vertex[i]->Jointweights.size(); j++) {
			
			int joint_num = vertex[i]->Jointweights[j]->first;
			float w = vertex[i]->Jointweights[j]->second;

			newPos = newPos + w * glm::vec3(skinningMatrix[joint_num] * glm::vec4(vertex[i]->position,1));
			//newNorm = newNorm + w * glm::vec3(skinningMatrix[joint_num] * glm::vec4(vertex[i]->normal, 0));
			newNorm = newNorm + w * glm::vec3(glm::inverse(glm::transpose(skinningMatrix[joint_num])) * glm::vec4(vertex[i]->normal, 0));
		}
		position[i] = newPos;
		normal[i] = glm::normalize(newNorm);
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

void Skin::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
	
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

Skin::~Skin() {
	for (int i = 0; i < vertex.size(); i++) {
		delete vertex[i];
	}
	for (int i = 0; i < Tsuit.size(); i++) {
		delete Tsuit[i];
	}
	// Delete the VBOs and the VAO.
	glDeleteBuffers(1, &VBO_positions);
	glDeleteBuffers(1, &VBO_normals);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
}