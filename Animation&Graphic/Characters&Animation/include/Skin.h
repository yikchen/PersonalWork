#include "Triangle.h"
#include "core.h"
#include "glm/gtx/euler_angles.hpp"
#include <vector>
#include "Tokenizer.h"
#include <iostream>
#include "Skeleton.h"
class Skin {
public:
	Skin();
	~Skin();
	bool Load(const char* file);
	void setSkeleton(Skeleton* s);
	void Update();
	void Draw(const glm::mat4& viewProjMtx, GLuint shader);
private:
	GLuint VAO;
	GLuint VBO_positions, VBO_normals, EBO;
	glm::mat4 model;
	glm::vec3 color;

	Skeleton* skel;
	std::vector<Vertex*> vertex;
	std::vector<Triangle*> Tsuit;
	std::vector<glm::mat4> bindMatrix;
	std::vector<glm::mat4> skinningMatrix;

	std::vector<glm::vec3> position;
	std::vector<glm::vec3> normal;
	std::vector<GLuint> indices;
};