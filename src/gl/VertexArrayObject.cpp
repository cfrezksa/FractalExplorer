#pragma once
#include "gl/VertexArrayObject.h"
#include "GLE/CAPI_GLE.h"

VertexArrayObject::VertexArrayObject() { 
	vaoId = 0;
}

VertexArrayObject::~VertexArrayObject() {
	destroy();

	GL_CHECK_ERROR;
}
void VertexArrayObject::create() {
	glGenVertexArrays(1, &vaoId);

	GL_CHECK_ERROR;
}

void VertexArrayObject::bind() { 

	glBindVertexArray(vaoId);
	GL_CHECK_ERROR;
}

void VertexArrayObject::release() { 

	glBindVertexArray(0);
	GL_CHECK_ERROR;
}

void VertexArrayObject::destroy() {
	glDeleteVertexArrays(1, &vaoId);
}
