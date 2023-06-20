#include "Keyframe.h"
#include <vector>
#include "Skin.h"
class Channel {
public:
	Channel();
	~Channel();
	std::string extrapolateIn;
	std::string extrapolateOut;
	float Evaluate(float time);
	bool Load(Tokenizer &token);
	void Precompute();
private:
	std::vector<Keyframe*> Keyframes;
};