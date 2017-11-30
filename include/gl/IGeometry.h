#pragma once
#include <string>
#include "Extras/OVR_Math.h"
using namespace std;
using namespace OVR;

class ShaderProgram;

class IGeometry {
public:
	virtual ~IGeometry() = 0 {};
	virtual void addDrawStyle(const string &name, ShaderProgram *shader) = 0;
	virtual void draw(const string &drawStyle) = 0;

	virtual void setTexture(const string &name, unsigned int id) = 0;
	virtual void setFloat(const string &name, float f) = 0;
	virtual void setInt(const string &name, int i) = 0;
	virtual void setBool(const string &name, bool b) = 0;
	virtual void setVector(const string &name, Vector2f v) = 0;
	virtual void setVector(const string &name, Vector3f v) = 0;
	virtual void setVector(const string &name, Vector4f v) = 0;
	virtual void setMatrix(const string &name, Matrix4f mat) = 0;

};
