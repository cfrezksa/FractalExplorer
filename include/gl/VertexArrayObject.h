#pragma once

#include "../GLE/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>
#include <iostream>

using namespace OVR;

class VertexArrayObject
{
public:
	VertexArrayObject();
	virtual ~VertexArrayObject();
	void create();
	void bind();
	void release();
	void destroy();

protected:
	GLuint        vaoId;
};
