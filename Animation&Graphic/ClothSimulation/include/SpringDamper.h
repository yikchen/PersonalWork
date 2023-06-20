#include "Particle.h"
class SpringDamper {
	float SpringConstant;
	float DampingConstant;
	float RestLength;
	Particle* p1;
	Particle* p2;
public:
	SpringDamper(Particle* a, Particle* b, float sc, float dc, float rl);
	~SpringDamper();
	void computeForce();
};