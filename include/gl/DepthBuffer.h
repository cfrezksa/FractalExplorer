#pragma once

#include "../GLE/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>
#include <iostream>

using namespace OVR;

class DepthBuffer
{
public:
	DepthBuffer(Sizei size, int sampleCount);
	virtual ~DepthBuffer();

	inline GLuint textureId() { return texId; }

protected:
	GLuint        texId;
};
