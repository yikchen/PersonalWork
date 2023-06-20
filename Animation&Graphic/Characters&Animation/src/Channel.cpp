#include "Channel.h"
#include <math.h>
Channel::Channel() {

}

float f_mod(float a, float b) {
	return a - b * floor(a / b);
}

float Channel::Evaluate(float Time) {
	float x = 0;


	//handle t that is before first keyframe:: extrapolateIn
	if (Keyframes[0]->Time > Time) {
		if (this->extrapolateIn == "constant") {
			return Keyframes[0]->Value;
		}
		else if (this->extrapolateIn == "linear") {
			float constant = Keyframes[0]->Value - Keyframes[0]->TangentIn * Keyframes[0]->Time;
			return Keyframes[0]->TangentIn * Time + constant;

		}
		else if (this->extrapolateIn == "cycle") { 
			float range = Keyframes[Keyframes.size() - 1]->Time - Keyframes[0]->Time;
			float shift_v = Keyframes[0]->Time;
			Time -= shift_v;
			Time = f_mod(Time, range) + shift_v;
		}
		else if(this->extrapolateIn == "cycle_offset") {
			float displacement = Keyframes[Keyframes.size() - 1]->Value - Keyframes[0]->Value;
			float range = Keyframes[Keyframes.size() - 1]->Time - Keyframes[0]->Time;
			int offset = (Time - Keyframes[0]->Time)/range -1;
			x = offset * displacement;
			float shift_v = Keyframes[0]->Time;
			Time -= shift_v;
			Time = f_mod(Time, range) + shift_v;
		}
		else {//handle case for bounce
			float range = Keyframes[Keyframes.size() - 1]->Time - Keyframes[0]->Time;
			int b_idx = 0;
			while (Time > Keyframes[Keyframes.size() - 1]->Time) {
				Time += range;
				b_idx++;
			}
			if (b_idx % 2 == 1) {
				Time = Keyframes[Keyframes.size() - 1]->Time - (Time - Keyframes[0]->Time);
			}
		}
	}
	//handle t that is after last keyframe
	else if (Keyframes[Keyframes.size() - 1]->Time < Time) {
		if (this->extrapolateOut == "constant") {
			return Keyframes[Keyframes.size()-1]->Value;
		}
		else if (this->extrapolateOut == "linear") {
			float constant = Keyframes[Keyframes.size() - 1]->Value - Keyframes[Keyframes.size() - 1]->TangentOut * Keyframes[Keyframes.size() - 1]->Time;
			return Keyframes[Keyframes.size() - 1]->TangentOut * Time + constant;
		}
		else if (this->extrapolateOut == "cycle") { 
			float range = Keyframes[Keyframes.size() - 1]->Time - Keyframes[0]->Time;
			float shift_v = Keyframes[0]->Time;
			Time -= shift_v;
			Time = f_mod(Time, range) + shift_v;
		
		}
		else if(this->extrapolateOut == "cycle_offset") {
			float displacement = Keyframes[Keyframes.size() - 1]->Value - Keyframes[0]->Value;
			float range = Keyframes[Keyframes.size() - 1]->Time - Keyframes[0]->Time;
			int offset = (Time - Keyframes[0]->Time) / range;
			x = offset * displacement;
			float shift_v = Keyframes[0]->Time;
			Time -= shift_v;
			Time = f_mod(Time, range) + shift_v;
		}
		else {// handle case for bounce
			float range = Keyframes[Keyframes.size() - 1]->Time - Keyframes[0]->Time;
			int b_idx = 0;
			while (Time > Keyframes[Keyframes.size() - 1]->Time) {
				Time -= range;
				b_idx++;
			}
			if (b_idx % 2 == 1) {
				Time = Keyframes[Keyframes.size() - 1]->Time - (Time - Keyframes[0]->Time);
			}
		}
	}
	int left = 0;
	int right = 0;
	int low = 0;
	int high = Keyframes.size() - 1;
	int mid = 0;
	//handle t that is exactly on a keyframe
	while (low <= high) {
		mid = low + (high - low) / 2;
		if (Keyframes[mid]->Time == Time) {
			return Keyframes[mid]->Value;
		}
		else if (Keyframes[mid]->Time < Time) {
			low = mid + 1;
			left = mid;
		}
		else {
			high = mid - 1;
			right = mid;
		}
	}

	float u = (Time - Keyframes[left]->Time) / (Keyframes[right]->Time - Keyframes[left]->Time); // converted t
	x += Keyframes[left]->D + u * (Keyframes[left]->C + u * (Keyframes[left]->B + u * (Keyframes[left]->A))); // value for DOF at time Time
	return x;
}

//load from the anim file
bool Channel::Load(Tokenizer& token) {
	//Parsing the file and set up the channel
	token.FindToken("extrapolate");

	std::string rulein = "";
	std::string ruleout = "";
	token.SkipWhitespace();
	char c = token.GetChar();
	while (c != ' ') {
		rulein.push_back(c);
		c = token.GetChar();
	}

	token.SkipWhitespace();
	c = token.GetChar();
	while (c != '\n') {
		ruleout.push_back(c);
		c = token.GetChar();
	}
	extrapolateIn = rulein;
	extrapolateOut = ruleout;
	

	token.FindToken("keys");
	int numKeys = token.GetInt();
	token.SkipLine();

	for (int i = 0; i < numKeys; i++) {
		Keyframe* ky = new Keyframe();
		Keyframes.push_back(ky);
		ky->Time = token.GetFloat();
		ky->Value = token.GetFloat();

		std::string tagin = "";
		std::string tagout = "";
		char c = token.GetChar();
		while (c != ' ') {
			tagin.push_back(c);
			c = token.GetChar();
		}
		token.SkipWhitespace();
		c = token.GetChar();
		while (c != '\n') {
			tagout.push_back(c);
			c = token.GetChar();
		}
		ky->RuleIn = tagin;
		ky->RuleOut = tagout;

	}

	this->Precompute();
	return true;
}

//compute tagent for each key and cubic coef for each key
void Channel::Precompute() {
	
	//compute tangent
	for (int i = 0; i < Keyframes.size(); i++) {
		//compute tagentIn
		if (Keyframes[i]->RuleIn == "flat") {
			Keyframes[i]->TangentIn = 0;
		}
		else if (Keyframes[i]->RuleIn == "linear") {
			if (i > 0) {
				Keyframes[i]->TangentIn = (Keyframes[i]->Value - Keyframes[i - 1]->Value) / (Keyframes[i]->Time - Keyframes[i - 1]->Time);
			}
			else {
				/*if (Keyframes.size() != 1) {
					Keyframes[i]->TangentIn = (Keyframes[i + 1]->Value - Keyframes[i]->Value) / (Keyframes[i + 1]->Time - Keyframes[i]->Time);
				}*/
				Keyframes[i]->TangentIn = 0;
			}
		}
		else { // rule for smooth
			if (i == 0) {
				/*if (Keyframes.size() != 1) {
					Keyframes[i]->TangentIn = (Keyframes[i + 1]->Value - Keyframes[i]->Value) / (Keyframes[i + 1]->Time - Keyframes[i]->Time);
				}*/
				Keyframes[i]->TangentIn = 0;
			}
			else if (i > 0 && i < Keyframes.size() - 1) {
				Keyframes[i]->TangentIn = (Keyframes[i+1]->Value - Keyframes[i - 1]->Value) / (Keyframes[i+1]->Time - Keyframes[i - 1]->Time);
			}
			else {
				Keyframes[i]->TangentIn = (Keyframes[i]->Value - Keyframes[i - 1]->Value) / (Keyframes[i]->Time - Keyframes[i - 1]->Time);
			}
		}


		//compute tagentOut
		if (Keyframes[i]->RuleOut == "flat") {
			Keyframes[i]->TangentOut = 0;
		}
		else if (Keyframes[i]->RuleOut == "linear") {
			if (i < Keyframes.size() - 1) {
				Keyframes[i]->TangentOut = (Keyframes[i + 1]->Value - Keyframes[i]->Value) / (Keyframes[i + 1]->Time - Keyframes[i]->Time);
			}
			else {
				/*if (Keyframes.size() != 1) {
					Keyframes[i]->TangentOut = (Keyframes[i]->Value - Keyframes[i - 1]->Value) / (Keyframes[i]->Time - Keyframes[i - 1]->Time);
				}*/
				Keyframes[i]->TangentOut = 0;
			}
		}
		else {//rule for smooth
			if (i == 0) {
				if (Keyframes.size() != 1) {
					Keyframes[i]->TangentOut = (Keyframes[i + 1]->Value - Keyframes[i]->Value) / (Keyframes[i + 1]->Time - Keyframes[i]->Time);
				}
			}
			else if (i > 0 && i < Keyframes.size() - 1) {
				Keyframes[i]->TangentOut = (Keyframes[i + 1]->Value - Keyframes[i - 1]->Value) / (Keyframes[i + 1]->Time - Keyframes[i - 1]->Time);
			}
			else {
				//Keyframes[i]->TangentOut = (Keyframes[i]->Value - Keyframes[i - 1]->Value) / (Keyframes[i]->Time - Keyframes[i - 1]->Time);
				Keyframes[i]->TangentOut = 0;
			}
		}
	}

	//compute cubic coef
	for (int i = 0; i < Keyframes.size()-1; i++) {
		float t_0 = Keyframes[i]->Time;
		float t_1 = Keyframes[i + 1]->Time;
		float p_0 = Keyframes[i]->Value;
		float p_1 = Keyframes[i + 1]->Value;
		float v_0 = Keyframes[i]->TangentOut;
		float v_1 = Keyframes[i + 1]->TangentIn;
		glm::vec4 coef;
		glm::mat4 H = glm::mat4(glm::vec4(2,-3,0,1), glm::vec4(-2,3,0,0), glm::vec4(1,-2,1,0), glm::vec4(1,-1,0,0));
		glm::vec4 P = glm::vec4(p_0,p_1,(t_1 - t_0)*v_0, (t_1 - t_0) * v_1);
		coef = H * P;

		Keyframes[i]->A = coef.x;
		Keyframes[i]->B = coef.y;
		Keyframes[i]->C = coef.z;
		Keyframes[i]->D = coef.w;
	}
	//Keyframes[Keyframes.size() -1]-> A,B,C,D= 
}

Channel::~Channel() {
	for (int i = 0; i < Keyframes.size(); i++) {
		delete Keyframes[i];
	}
}