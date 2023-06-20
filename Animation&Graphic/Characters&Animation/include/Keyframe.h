#include <string>
class Keyframe {
public:
	Keyframe();
	~Keyframe();



	float Time;
	float Value;
	float TangentIn;
	float TangentOut;
	std::string RuleIn;
	std::string RuleOut;
	float A, B, C, D;
};