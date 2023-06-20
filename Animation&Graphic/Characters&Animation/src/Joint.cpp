#include "Joint.h"
#include <glm/gtx/string_cast.hpp>

Joint::Joint() {
	parent = NULL;
	box = NULL;
	joint_idx = joint_num++;
	name = "";
	DOF* x = new DOF;
	DOF* y = new DOF;
	DOF* z = new DOF;

	DOFs[0] = x;
	DOFs[1] = y;
	DOFs[2] = z;
	
	childJoint = new std::vector<Joint*>();

	world_matrix = glm::mat4(1.0f);
	local_matrix = glm::mat4(1.0f);
}

int Joint::joint_num = 0;

void Joint::Update(glm::mat4 &parent) {
	//std::cout << "stored value" << "x:" << DOFs[0]->GetValue() << " " << "y:" << DOFs[1]->GetValue() << " " << "z:" << DOFs[2]->GetValue() << std::endl;
	glm::mat4 R_x = glm::rotate(DOFs[0]->GetValue(), glm::vec3(1, 0, 0));
	glm::mat4 R_y = glm::rotate(DOFs[1]->GetValue(), glm::vec3(0, 1, 0));
	glm::mat4 R_z = glm::rotate(DOFs[2]->GetValue(), glm::vec3(0, 0, 1));
	local_matrix = glm::mat4(glm::vec4(1,0,0,0), glm::vec4(0,1,0,0), glm::vec4(0,0,1,0), glm::vec4(Offset,1)) *  R_z * R_y * R_x;
	world_matrix = parent * local_matrix;
	for (int i = 0; i < childJoint->size(); i++) {
		childJoint->at(i)->Update(world_matrix);
	}
}

bool Joint::Load(Tokenizer& token) {
	token.FindToken("{");
	while (1) {
		char temp[256];
		token.GetToken(temp);
		if (strcmp(temp, "offset") == 0) {
			Offset.x = token.GetFloat();
			Offset.y = token.GetFloat();
			Offset.z = token.GetFloat();
			//std::cout << "Offset value " << "x:" << Offset.x << " " << "y:" << Offset.y << " " << "z:" << Offset.z << std::endl;

		}
		else if (strcmp(temp, "boxmin") == 0) {
			boxmin.x = token.GetFloat();
			boxmin.y = token.GetFloat();
			boxmin.z = token.GetFloat();
		}// Check for other tokens
		else if (strcmp(temp, "boxmax") == 0) {
			boxmax.x = token.GetFloat();
			boxmax.y = token.GetFloat();
			boxmax.z = token.GetFloat();
	
		}
		else if (strcmp(temp, "rotxlimit") == 0) {
			float min = token.GetFloat();
			float max = token.GetFloat();
			//std::cout << "x min max" << min << " " << max << std::endl;
			DOFs[0]->SetMinMax(min, max);
		}
		else if (strcmp(temp, "rotylimit") == 0) {
			float min = token.GetFloat();
			float max = token.GetFloat();
			//std::cout << "y min max" << min << " " << max << std::endl;
			DOFs[1]->SetMinMax(min, max);
		}
		else if (strcmp(temp, "rotzlimit") == 0) {
			float min = token.GetFloat();
			float max = token.GetFloat();
			//std::cout << "z min max" << min << " " << max << std::endl;
			DOFs[2]->SetMinMax(min, max);
		}
		else if (strcmp(temp, "pose") == 0) {
			float x = token.GetFloat();
			float y = token.GetFloat();
			float z = token.GetFloat();
			DOFs[0]->SetValue(x);
			DOFs[1]->SetValue(y);
			DOFs[2]->SetValue(z);
			//std::cout << std::endl;
			//std::cout << "read value " << "x:" << x << " " << "y:" << y << " " << "z:" << z << std::endl;
			//std::cout << "stored value " << "x:" << DOFs[0]->GetValue() << " " << "y:" << DOFs[1]->GetValue() << " " << "z:" << DOFs[2]->GetValue() << std::endl;
			//std::cout << "------------------------------------" << std::endl;

		}
		else if (strcmp(temp, "balljoint") == 0) {
			token.SkipWhitespace();
			std::string name = "";
			char c = token.GetChar();
			while (c != ' ') {
				name.push_back(c);
				c = token.GetChar();
			}
			Joint* jnt = new Joint;
			jnt->name = name;
			jnt->Load(token);
			AddChild(*jnt);
		}
		else if (strcmp(temp, "}") == 0) {
			return true;
		}
		else {
			token.SkipLine(); // Unrecognized token
		}
	}
}

void Joint::AddChild(Joint &child) {
	//std::cout << "x:" << child.DOFs[0]->GetValue() << " " << "y:" << child.DOFs[1]->GetValue() << " " << "z:" << child.DOFs[2]->GetValue() << std::endl;
	child.parent = this;
	this->childJoint->push_back(&child);
}

void Joint::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
	box = new Cube(boxmin,boxmax);
	glm::mat4 vw = viewProjMtx * world_matrix;
	box->draw(vw, shader);
	delete box;
}

Joint::~Joint() {
	for (int i = 0; i < 3; i++) {
		delete DOFs[i];
	}
	delete [] DOFs;
	for (int i = 0; i < childJoint->size(); i++) {
		this->childJoint->at(i)->~Joint();
	}
	delete childJoint;
}