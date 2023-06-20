#include "SpringDamper.h"
#include <iostream>
SpringDamper::SpringDamper(Particle* a, Particle* b, float sc, float dc, float rl) {
	p1 = a;
	p2 = b;
	SpringConstant = sc;
	DampingConstant = dc;
	RestLength = rl;
}

void SpringDamper::computeForce() {
	float l = glm::length(p2->getPos() - p1->getPos());
	glm::vec3 e = glm::normalize(p2->getPos() - p1->getPos());

	glm::vec3 v_clos = (p1->getVec() - p2->getVec()) * e;
	glm::vec3 f = -SpringConstant * (RestLength - l) - DampingConstant * v_clos;

	p1->ApplyForce(f * e);
	p2->ApplyForce(-(f * e));
}

SpringDamper::~SpringDamper() {

}