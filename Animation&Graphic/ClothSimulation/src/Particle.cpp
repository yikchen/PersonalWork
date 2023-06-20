#include "Particle.h"
#include <iostream>
Particle::Particle(glm::vec3 pos, float m) {
	fix = false;
	deafault_pos = pos;
	Position = pos;
	p_pos = glm::vec3(0, -1, 0);
	p_norm = glm::vec3(0, 1, 0);
	Mass = m;
	restitution = 0.05;
	mu = 0.75;
	Force = glm::vec3(0, -9.81, 0) * Mass;
	Velocity = glm::vec3(0);
}

void Particle::ApplyForce(glm::vec3 f) {
	Force += f;
}

void Particle::ApplyImpulse(glm::vec3 j) {
	Velocity += j / Mass;
}

void Particle::Integrate(float deltaT) {
	if (!fix) {
		glm::vec3 prev_pos = Position;
		glm::vec3 accel = (1 / Mass) * Force;
		Velocity += accel * deltaT;
		Position += Velocity * deltaT;
		if (Position.y < -1) {
			float d_a = glm::dot((prev_pos - p_pos), p_norm);
			float d_b = glm::dot((Position - p_pos), p_norm);
			glm::vec3 intersect = (d_a * Position - d_b * prev_pos) / (d_a - d_b);
			float v_clos = glm::dot(Velocity, p_norm);
			glm::vec3 j = -(1 + restitution) * Mass * v_clos * p_norm;
			ApplyImpulse(j);
			glm::vec3 v_tan = Velocity - glm::dot(Velocity, p_norm) * p_norm;
			glm::vec3 f_j = -glm::normalize(v_tan) * mu * glm::length(j);
			ApplyImpulse(f_j);
			Position = intersect + glm::vec3(0,0.001,0);
		}

		Force = glm::vec3(0, -9.81, 0) * Mass;
	}
}

void Particle::setPos(glm::vec3 pos) {
	Position = deafault_pos + pos;
}

void Particle::setfix() {
	fix = true;
}

void Particle::releasefix() {
	fix = false;
}

glm::vec3 Particle::getPos() {
	return Position;
}

glm::vec3 Particle::getVec() {
	return Velocity;
}

glm::vec3 Particle::getFor() {
	return Force;
}

Particle::~Particle() {

}