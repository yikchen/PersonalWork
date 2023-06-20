#include "Channel.h"

class AnimationClip {
public:
	AnimationClip();
	~AnimationClip();
	bool Load(const char* filename);
	void Update(float t, std::vector<Joint*> Pose);
	//void Evaluate(float time, )
private:
	std::vector<Channel*> Channels;
	float timeStart;
	float timeEnd;
};