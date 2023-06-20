#include "Quaternion.h"
#include <iostream>

Quaternion::Quaternion() {
	qua = glm::vec4(1, 0, 0, 0);
	translation = glm::vec3(0);
	axis = new Quaternion(glm::vec4(1.0f));
}

Quaternion::Quaternion(glm::vec4 q) {
	qua = glm::vec4(1, 0, 0, 0);
	translation = glm::vec3(0);
	axis = NULL;
}

void Quaternion::setTranslation(glm::vec3 tran) {
	translation = tran;
}

void Quaternion::negate() {
	glm::vec4 temp = glm::vec4(-qua.x, -qua.y, -qua.z, -qua.w);
	qua = glm::normalize(temp);
}

float Quaternion::Dot(Quaternion& q0, Quaternion& q1) {
	glm::vec4 p = q0.qua;
	glm::vec4 q = q1.qua;

	float dot = p.x * q.x + p.y * q.y + p.z * q.z + p.w * q.w;
	return dot;
}


void Quaternion::Multiply(Quaternion& q0, Quaternion& q1) {
	glm::vec4 p = q0.qua;
	glm::vec4 q = q1.qua;

	qua.x = p.x * q.x - p.y * q.y - p.z * q.z - p.w * q.w;
	qua.y = p.x * q.y + p.y * q.x + p.z * q.w - p.w * q.z;
	qua.z = p.x * q.z + p.z * q.x - p.y * q.w + p.w * q.y;
	qua.w = p.x * q.w + p.w * q.x + p.y * q.z - p.z * q.y;
	qua = glm::normalize(qua);
}

void Quaternion::MakeRotateX(float t) {
	t = t * glm::pi<float>() / 180.0f;
	float factor = glm::sin(t / 2.0f);
	float re = glm::cos(t / 2.0f);

	glm::vec4 x = glm::vec4(re, 1* factor, 0, 0);
	axis->qua = glm::normalize(x);
	Multiply(*axis, *this);
}

void Quaternion::MakeRotateY(float t) {
	t = t * glm::pi<float>() / 180.0f;
	float factor = glm::sin(t / 2.0f);
	float re = glm::cos(t / 2.0f);

	glm::vec4 y = glm::vec4(re, 0, 1 * factor, 0);
	axis->qua = glm::normalize(y);
	Multiply(*axis, *this);
}

void Quaternion::MakeRotateZ(float t) {
	t = t * glm::pi<float>() / 180.0f;
	float factor = glm::sin(t / 2.0f);
	float re = glm::cos(t / 2.0f);

	glm::vec4 z = glm::vec4(re, 0, 0, 1 * factor);
	axis->qua = glm::normalize(z);
	Multiply(*axis, *this);
}

void Quaternion::ToMatrix(glm::mat4& m) {
	glm::vec4 x = glm::vec4((1 - 2 * glm::pow(qua.z, 2) - 2 * glm::pow(qua.w, 2)), (2 * qua.y * qua.z + 2 * qua.x * qua.w), (2 * qua.y * qua.w - 2 * qua.x * qua.z),0);
	glm::vec4 y = glm::vec4((2*qua.y*qua.z - 2*qua.x*qua.w), (1- 2* glm::pow(qua.y,2) - 2*glm::pow(qua.w,2)), (2*qua.z * qua.w + 2* qua.x*qua.y), 0);
	glm::vec4 z = glm::vec4((2*qua.y*qua.w + 2*qua.x*qua.z), (2*qua.z*qua.w - 2*qua.x*qua.y), (1-2*glm::pow(qua.y,2) - 2*glm::pow(qua.z,2)), 0);
	glm::vec4 w = glm::vec4(translation, 1);
	m = glm::mat4(x, y, z, w);
}

void Quaternion::Slerp(float t, Quaternion& q0, Quaternion& q1) {
	float theta = glm::acos(Dot(q0, q1));
	if (Dot(q0, q1) >= 1) {
		theta = 0.0f;
	}
	if (theta != 0) {
		glm::vec4 tmp = ((glm::sin((1 - t) * theta) / glm::sin(theta))) * q0.qua + (glm::sin(t * theta) / glm::sin(theta)) * q1.qua;
		this->qua = glm::normalize(tmp);
	}
	
	glm::vec3 tmp1 = (1 - t) * q0.translation + t * q1.translation;
	this->translation = tmp1;
}

glm::vec4 Quaternion::Slerp(float t, glm::vec4 q0, glm::vec4 q1) {
	
	float theta = glm::acos(glm::dot(q0,q1));

	if (glm::dot(q0, q1) >= 1) {
		theta = 0.0f;
	}
	if (theta != 0) {
		glm::vec4 tmp = ((glm::sin((1 - t) * theta) / glm::sin(theta))) * q0 + (glm::sin(t * theta) / glm::sin(theta)) * q1;
		return glm::normalize(tmp);
	}
	return glm::normalize(q0);
}

void Quaternion::CatmullRom(float t, Quaternion& q0, Quaternion& q1, Quaternion& q2, Quaternion& q3) {
	glm::vec4 q10 = Slerp( t + 1,q0.qua, q1.qua);
	glm::vec4 q11 = Slerp(t, q1.qua, q2.qua);
	glm::vec4 q12 = Slerp(t - 1, q2.qua, q3.qua);
	glm::vec4 q20 = Slerp((t+1) / 2, q10, q11);
	glm::vec4 q21 = Slerp(t / 2, q11, q12);
	this->qua = Slerp(t, q20, q21);


	glm::vec3 tmp1 = (1 - t) * q1.translation + t * q2.translation;
	this->translation = tmp1;
}

Quaternion::~Quaternion() {
	if (axis != NULL) {
		delete axis;
	}
}