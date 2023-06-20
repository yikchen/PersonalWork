#include "SpringDamper.h"
class Triangle {
	glm::vec3 V_sur;
	glm::vec3 normal;
	float density;
	float drag_coe;
	float area_0;
	Particle* p1;
	Particle* p2;
	Particle* p3;
public:
	Triangle(Particle* a, Particle* b, Particle* c,float d, float coe);
	glm::vec3 V_air;
	glm::vec3 getNorm();
	void computeForce();
	~Triangle();
};