#pragma once

#include "../GLE/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>
#include <iostream>
#include <string>

using namespace std;
using namespace OVR;

class Shader;

class ShaderProgram
{
public:
	ShaderProgram(const string &vs, const string& fs);
	virtual ~ShaderProgram();
	void create();
	void activate();
	void deactivate();
	int attributeLocation(const string &attribName);
	void enableAttributeArray(int varLoc);
	void setAttributeArray(int varLoc, GLenum type, const void* offset, int tupleSize, int stride);
	
	int uniformLocation(const string &name);
	void setUniformValue(int uniLoc, GLuint texActive);
	void setUniformValue(int uniLoc, float value);
	void setUniformValue(int uniLoc, Vector2f value);
	void setUniformValue(int uniLoc, Vector3f value);
	void setUniformValue(int uniLoc, Vector4f value);
	void setUniformValue(int uniLoc, Matrix4f value);
	void setUniformValue(int uniLoc, Matrix3f value);
	void setUniformValue(int uniLoc, int value);
	void setUniformValue(int uniLoc, bool value);
protected:
	static void checkLogBuffers(int length);
	static bool checkShaderLog(GLuint shader);
	static bool checkProgramLog(GLuint shader);

protected:
	string vertexShaderFile;
	string fragmentShaderFile;

	GLuint        progId;

	static char *infoLog;
	static int   infoLogLength;
};
