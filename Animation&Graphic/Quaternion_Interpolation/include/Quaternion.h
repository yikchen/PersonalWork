#include "core.h"

class Quaternion {
public:
	glm::vec4 qua;
	glm::vec3 translation;
	Quaternion();
	Quaternion(glm::vec4 q);
	~Quaternion();
	float Dot(Quaternion& q0, Quaternion& q1);
	void setTranslation(glm::vec3 translation);
	void negate();
	void Multiply(Quaternion& q0, Quaternion& q1);
	void MakeRotateX(float t);
	void MakeRotateY(float t);
	void MakeRotateZ(float t);
	void ToMatrix(glm::mat4& m);
	void Slerp(float t, Quaternion& q0, Quaternion& q1);
	glm::vec4 Slerp(float t, glm::vec4 q0, glm::vec4 q1);
	void CatmullRom(float t, Quaternion& q0, Quaternion& q1, Quaternion& q2, Quaternion& q3);
private:
	Quaternion* axis;
};