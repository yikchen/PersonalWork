#include "Triangle.h"
#include <iostream>
Triangle::Triangle(Particle* a, Particle* b, Particle* c, float d, float coe) {
	p1 = a;
	p2 = b;
	p3 = c;
	V_air = glm::vec3(0,0,0);
	density = d;
	drag_coe = coe;
}

glm::vec3 Triangle::getNorm() {
	return normal;
}
void Triangle::computeForce() {
	
	V_sur = 0.33333f * (p1->getVec() + p2->getVec() + p3->getVec());
	normal = glm::normalize(glm::cross((p2->getPos() - p1->getPos()), (p3->getPos() - p1->getPos())));
	area_0 = 0.5f * glm::length(glm::cross((p2->getPos() - p1->getPos()), (p3->getPos() - p1->getPos())));
	
	glm::vec3 v = V_sur - V_air;
	if (glm::length(v) == 0) {
		p1->ApplyForce(glm::vec3(0));
		p1->ApplyForce(glm::vec3(0));
		p1->ApplyForce(glm::vec3(0));
	}
	else {
		float area = area_0 * (glm::dot(glm::normalize(v), normal));
		float v_sq = glm::pow(glm::length(v), 2);
		glm::vec3 f_aero = -0.5f * density * v_sq * drag_coe * area * normal;

		p1->ApplyForce(0.33333f * f_aero);
		p2->ApplyForce(0.33333f * f_aero);
		p3->ApplyForce(0.33333f * f_aero);
	}
	
}

Triangle::~Triangle() {

}