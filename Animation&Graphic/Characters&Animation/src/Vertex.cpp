#include "Vertex.h"
Vertex::Vertex() {
	position = glm::vec3(0, 0, 0);
	normal = glm::vec3(0, 0, 0);
}

void Vertex::setJointWeight(int joint, float weight) {
	std::pair<int, float>* p = new std::pair<int, float>();
	p->first = joint;
	p->second = weight;
	Jointweights.push_back(p);
}

Vertex::~Vertex() {
	for (int i = 0; i < Jointweights.size(); i++) {
		delete Jointweights[i];
	}
}