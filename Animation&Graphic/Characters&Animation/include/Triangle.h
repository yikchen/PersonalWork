#include "Vertex.h"
class Triangle {
public:
	Triangle();
	~Triangle();
	void setv0(Vertex* v);
	void setv1(Vertex* v);
	void setv2(Vertex* v);
	void Draw();
	Vertex* getVertex0();
	Vertex* getVertex1();
	Vertex* getVertex2();
	//void Draw();
private:
	Vertex* v0;
	Vertex* v1;
	Vertex* v2;
};