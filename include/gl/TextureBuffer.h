#pragma once

#include "../GLE/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>
#include <iostream>

using namespace OVR;

class DepthBuffer;

class TextureBuffer
{           

public:
	TextureBuffer(
		ovrSession session, 
		bool rendertarget, 
		bool displayableOnHmd, 
		Sizei size, 
		int mipLevels, 
		unsigned char * data);


	TextureBuffer(
		Sizei size,
		int mipLevels,
		unsigned char * data,
		bool renderTarget);

	virtual ~TextureBuffer();

	inline ovrTextureSwapChain TextureChain() {
		return texChain;
	}

	Sizei GetSize() const;

	void SetAndClearRenderSurface(DepthBuffer* dbuffer);

	void UnsetRenderSurface();

	void Commit();

	inline GLuint textureId() { return texId; }

protected:
	ovrSession          Session;
	ovrTextureSwapChain texChain;
	GLuint              texId;
	GLuint              fboId;
	Sizei				texSize;
};
