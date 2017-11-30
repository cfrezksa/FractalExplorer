#include "GLE/CAPI_GLE.h"
#include "gl/DrawCall.h"
#include "gl/VertexArrayObject.h"
#include "gl/ShaderProgram.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

DrawCall::DrawCall() {

	vao = NULL;
	shader = NULL;

}

void DrawCall::activate() {

	GL_CHECK_ERROR;

	assert(NULL != vao);
	assert(NULL != shader);

	//assert(vao->isCreated());

	vao->bind();
	GL_CHECK_ERROR;

	shader->activate();
	GL_CHECK_ERROR;

}

void DrawCall::deactivate() {

	assert(NULL != vao);
	assert(NULL != shader);

	vao->release();
	shader->deactivate();
}


void DrawCall::prepare(ShaderProgram *prog) {

	GL_CHECK_ERROR;
	shader = prog;
	shader->create();

	GL_CHECK_ERROR;
	vao = new VertexArrayObject();

	GL_CHECK_ERROR;
	vao->create();

	GL_CHECK_ERROR;
	vao->bind();
	shader->activate();

	GL_CHECK_ERROR;
}

DrawCall::~DrawCall() {
	if (NULL != vao) vao->destroy();
	delete vao;
}
