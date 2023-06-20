#include <stack>
#include "Skeleton.h"


Skeleton::Skeleton() {
	root = NULL;
}

bool Skeleton::Load(const char *file) {
	Tokenizer token;
	token.Open(file);
	token.FindToken("balljoint");
	token.SkipWhitespace();
	std::string name = "";
	char c = token.GetChar();
	while (c != ' ') {
		name.push_back(c);
		c = token.GetChar();
	}
	
	
	//Parse Tree
	root = new Joint;
	root->name = name;
	root->Load(token);
	
	//Finish
	token.Close();
	//Add all created joints
	std::stack<Joint*> toADD;
	toADD.push(root);
	while (!toADD.empty()) {
		Joint* cur = toADD.top();
		toADD.pop();
		joints[cur->name] = cur;
		for (int i = 0; i < cur->childJoint->size(); i++) {
			Joint* children = cur->childJoint->at(i);
			toADD.push(children);
		}
	}
	return true;
}

void Skeleton::Update() {
	root->Update(glm::mat4(1.0f));
}

void Skeleton::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
	std::map<std::string, Joint*>::iterator it = joints.begin();
	while (it != joints.end()) {
		it->second->Draw(viewProjMtx, shader);
		++it;
	}

}

glm::mat4 Skeleton::GetWorldMatrix(int joint) {
	std::map<std::string, Joint*>::iterator it = joints.begin();
	while (it != joints.end()) {
		if (it->second->joint_idx == joint) {
			return it->second->world_matrix;
		}
		++it;
	}
	return glm::mat4(1.0f);
}


Skeleton::~Skeleton() {
	delete root;
}