#include "Triangle.h"
#include <vector>
class Cloth {
	int row;
	int col;
	std::vector<Particle*> vertices;
	std::vector<SpringDamper*> springs;
	std::vector<Triangle*> triangles;
	
	float stiffness;
	float d_stiffness;

	std::vector<glm::vec3> normal;

	GLuint VAO;
	GLuint VBO_positions, VBO_normals, EBO;
	glm::mat4 model;
	glm::vec3 color;
	
	
	
	
public:
	std::vector<glm::vec3> position;
	std::vector<GLuint> indices;
	Cloth(float size, float wps,int x, int y, float stiff, float d_stiff);
	void Draw(const glm::mat4& viewProjMtx, GLuint shader);
	void Update(float deltaT, glm::vec3 pos, glm::vec3 wind);
	~Cloth();
};