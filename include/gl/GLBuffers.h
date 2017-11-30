#pragma once

#include "../GLE/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>
#include <iostream>


template<GLenum T, GLenum U> class GLBuffer
{
	GLuint buffer;

public:
	GLBuffer(void* vertices, size_t size)
	{
		glGenBuffers(1, &buffer);
		assert(buffer != 0);
		glBindBuffer(T, buffer);
		glBufferData(T, size, vertices, U);
		GL_CHECK_ERROR;
	}

	~GLBuffer()
	{
		if (buffer)
		{
			glDeleteBuffers(1, &buffer);
			buffer = 0;
		}
	}

	void bind() {
		glBindBuffer(T, buffer);
		GL_CHECK_ERROR;
	}
};

typedef GLBuffer<GL_ARRAY_BUFFER, GL_STATIC_DRAW> VertexBuffer;

typedef GLBuffer<GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW> IndexBuffer;

