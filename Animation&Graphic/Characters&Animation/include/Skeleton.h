#include "Joint.h"
#include <map>
class Skeleton {
public:
	Skeleton();
	~Skeleton();
	bool Load(const char *file);
	glm::mat4 GetWorldMatrix(int joint);
	void Update();
	void Draw(const glm::mat4& viewProjMtx, GLuint shader);
	std::map <std::string, Joint*> joints;
private:
	Joint* root;
};