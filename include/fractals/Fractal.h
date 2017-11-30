#pragma once

#include "../GLE/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "../gl/ShaderProgram.h"
#include "../gl/GLBuffers.h"
#include "../gl/Geometry.h"

class Fractal : public Geometry {

public:
	Fractal();
	Fractal(const string &filename);
	virtual ~Fractal();

	void setType(const string &type);
	void write(const string &filename);
	void read(const string &filename);
	void draw();

private:
	static VertexBuffer *staticVertexBuffer;
	static void init();
	static int numFloatsPerVertex;

	static int TypeTokenId(const string &token);
	static const int NUM_TYPE_TOKENS;
	static vector<string> typeTokens;
	static map<string, ShaderProgram *> shaders;
	string drawStyle;
protected:
	void commonConstructor();
};