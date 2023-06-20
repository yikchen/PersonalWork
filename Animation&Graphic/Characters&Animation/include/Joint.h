#include "core.h"
#include "glm/gtx/euler_angles.hpp"
#include <vector>
#include "DOF.h"
#include "Tokenizer.h"
#include "Cube.h"
#include <iostream>
#include <string>
class Joint {
public:
	static int joint_num;
	std::string name;
	int joint_idx;
	Joint();
	~Joint();
	void Update(glm::mat4 &parent);
	bool Load(Tokenizer &t);
	void AddChild(Joint &child);
	void Draw(const glm::mat4& viewProjMtx, GLuint shader);
	std::vector<Joint*>* childJoint; 
	DOF** DOFs = new DOF*[3];
	Joint* parent;

	glm::mat4 world_matrix;
	glm::mat4 local_matrix;
	glm::vec3 Offset = glm::vec3(0, 0, 0);
private:
	Cube* box;
	glm::vec3 boxmin = glm::vec3(-0.1f,-0.1f,-0.1f);
	glm::vec3 boxmax = glm::vec3(0.1f,0.1f,0.1f);
};