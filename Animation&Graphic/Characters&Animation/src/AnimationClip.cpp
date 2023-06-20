#include "AnimationClip.h"

AnimationClip::AnimationClip() {

}

void AnimationClip::Update(float t, std::vector<Joint*> Pose) {
	for (int i = 0; i < Pose.size(); i++) {
		int idx = Pose[i]->joint_idx;
		if (idx == 0) {
			Pose[i]->Offset.x = Channels[0]->Evaluate(t);
			Pose[i]->Offset.y = Channels[1]->Evaluate(t);
			Pose[i]->Offset.z = Channels[2]->Evaluate(t);
		}
		Pose[i]->DOFs[0]->SetValue(Channels[idx*3 +3]->Evaluate(t));
		Pose[i]->DOFs[1]->SetValue(Channels[idx*3 +4]->Evaluate(t));
		Pose[i]->DOFs[2]->SetValue(Channels[idx*3 +5]->Evaluate(t));
	}
}
bool AnimationClip::Load(const char* filename) {
	std::cout << "We are here in animation load" << std::endl;
	
	Tokenizer token;
	token.Open(filename);
	token.FindToken("range");
	timeStart = token.GetFloat();
	timeEnd = token.GetFloat();

	token.FindToken("numchannels");
	int numChannels = token.GetInt();
	for (int i = 0; i < numChannels; i++) {
		//create channel and add it to the vector
		token.FindToken("channel");
		Channel* chn = new Channel();
		Channels.push_back(chn);
		chn->Load(token);
	}

	token.Close();
	return true;
}

AnimationClip::~AnimationClip() {
	for (int i = 0; i < Channels.size(); i++) {
		delete Channels[i];
	}
}