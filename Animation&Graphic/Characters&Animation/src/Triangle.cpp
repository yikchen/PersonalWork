#include "Triangle.h"
Triangle::Triangle() {
	v0 = NULL;
	v1 = NULL;
	v2 = NULL;
}

Vertex* Triangle::getVertex0() {
	return v0;
}


Vertex* Triangle::getVertex1() {
	return v1;
}


Vertex* Triangle::getVertex2() {
	return v2;
}

void Triangle::setv0(Vertex* v) {
	v0 = v;
}

void Triangle::setv1(Vertex* v) {
	v1 = v;
}
void Triangle::setv2(Vertex* v) {
	v2 = v;
}

void Triangle::Draw() {

}

Triangle::~Triangle() {

}