#include "core.h"
#include <vector>
#include <utility>
class Vertex {
public:
	Vertex();
	~Vertex();
	void setJointWeight(int i, float weight);
	glm::vec3 position;
	glm::vec3 normal;
	std::vector<std::pair<int,float>*> Jointweights;
};