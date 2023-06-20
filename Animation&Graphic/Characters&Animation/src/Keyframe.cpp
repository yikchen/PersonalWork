#include "Keyframe.h"

Keyframe::Keyframe() {
	Time = 0;
	Value = 0;
	TangentIn = 0;
	TangentOut = 0;
	RuleIn = "";
	RuleOut = "";
	A = 0;
	B = 0;
	C = 0;
	D = 0;
}

Keyframe::~Keyframe() {

}