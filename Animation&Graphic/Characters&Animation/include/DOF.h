class DOF {
public:
	DOF();
	~DOF();
	void SetValue(float val);
	float GetValue();
	float GetMax();
	float GetMin();
	void SetMinMax(float m, float M);
	float value;
private:
	float max;
	float min;
};