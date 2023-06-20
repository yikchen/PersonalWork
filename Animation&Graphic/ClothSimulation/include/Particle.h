#include "core.h"
class Particle {
public:
	Particle(glm::vec3 pos, float m);
	~Particle();
	void setfix();
	void releasefix();
	void setPos(glm::vec3 pos);
	glm::vec3 getPos();
	glm::vec3 getVec();
	glm::vec3 getFor();
	void ApplyForce(glm::vec3 f);
	void Integrate(float deltaTime);
	void ApplyImpulse(glm::vec3 j);
	glm::vec3 Force;
private:
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec3 deafault_pos;
	glm::vec3 p_pos;
	glm::vec3 p_norm;
	float Mass;
	float restitution;
	float mu;
	bool fix;
};