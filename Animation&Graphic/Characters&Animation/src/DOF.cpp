#include "DOF.h"
#include <iostream>

DOF::DOF() {
	value = 0.0f;
	max = 100000.0f;
	min = -10000.0f;
}

float DOF::GetValue() {
	return value;
}

float DOF::GetMax() {
	return max;
}

float DOF::GetMin() {
	return min;
}

void DOF::SetValue(float val) {
	
	if (val < min) {
		value = min;
	}
	else if (val > max) {
		value = max;
	}
	else {
		value = val;
	}
	//std::cout << "min" << min << " max" << max <<" value"<< value << std::endl;
}

void DOF::SetMinMax(float m, float M) {
	min = m;
	max = M;
}

DOF::~DOF() {

}