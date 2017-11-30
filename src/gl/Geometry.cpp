#include "core/globalDefs.h"
#include "gl/Geometry.h"
#include "gl/DrawCall.h"
#include "gl/ShaderProgram.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;


Geometry::Geometry(VertexBuffer *vb, GLenum primType, GLint primStart, GLsizei primCount, GLint s)
{
	vertexBuffer = vb;
	numPrimitives = primCount;
	primitiveType = primType;
	primitiveStart = primStart;
	stride = s;

}

Geometry::Geometry()
{
	vertexBuffer = NULL;
	numPrimitives = 0;
	primitiveStart = 0;
	stride = 0;
	primitiveType = GL_TRIANGLES;

}

Geometry::~Geometry(void)
{

}

void Geometry::addDrawStyle(const string &name, ShaderProgram *shader) {

	auto it = drawStyles.find(name);
	if (it != drawStyles.end()) {
		auto call = (*it).second;
		drawStyles.erase(it);
		delete call;
	}
	
	DrawCall *drawCall = new DrawCall();
	drawCall->prepare(shader);
	prepareDrawStyle(drawCall);
	drawStyles[name] = drawCall;
	
	GL_CHECK_ERROR;
}

void Geometry::prepareDrawStyle(DrawCall *drawCall) {

	ShaderProgram *shader = drawCall->shaderProgram();

	bindGeometry();

	for each (auto entry in bufferOffsets)
	{
		string attribName = (entry.first);

		int varLoc = shader->attributeLocation(attribName);

		assert(varLoc >= 0);

		if (varLoc != -1) {
			void *offset = (void*)BUFFER_OFFSET(bufferOffsets[attribName]);
			shader->enableAttributeArray(varLoc);
			shader->setAttributeArray(varLoc, GL_FLOAT, offset, 3, stride);
		}
	}

	GL_CHECK_ERROR;
}

void Geometry::bindGeometry() {

	vertexBuffer->bind();

	GL_CHECK_ERROR;
}

template<class T> void setParams(const map<string, T> &map, ShaderProgram *shader) {

	GL_CHECK_ERROR;


	for each (auto entry in map) {

		const string& name = entry.first;

		GL_CHECK_ERROR;
		T value = entry.second;
		int uniLoc = shader->uniformLocation(name);
		
		GL_CHECK_ERROR;

		if (uniLoc != -1) {
			shader->setUniformValue(uniLoc, value);
			GL_CHECK_ERROR;
		}
		else {
			OVR_DEBUG_LOG((name + ": parameter not found!}\n").c_str());
		}
	}

	GL_CHECK_ERROR;
}

unsigned int Geometry::setTexParams(ShaderProgram *shader) {

	unsigned int texActive = 0;

	map<string, unsigned int>::const_iterator it = textures.begin();

	while (it != textures.end()) {

		const string& name = (*it).first;
		unsigned int value = (*it).second;
		int uniLoc = shader->uniformLocation(name);

		if (uniLoc != -1) {
			glActiveTexture(GL_TEXTURE0 + texActive);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, value);
			shader->setUniformValue(uniLoc, texActive);
			texActive++;
		}

		it++;
	}

	GL_CHECK_ERROR;

	return texActive;
}

void Geometry::setUniforms(ShaderProgram *shader) {

	numActiveTextures = setTexParams(shader);
	setParams<float>(floatParams, shader);
	setParams<Vector2f>(vec2Params, shader);
	setParams<Vector3f>(vec3Params, shader);
	setParams<Vector4f>(vec4Params, shader);
	setParams<Matrix4f>(mat4x4Params, shader);
	setParams<Matrix3f>(mat3x3Params, shader);
	setParams<int>(intParams, shader);
	setParams<bool>(boolParams, shader);

	GL_CHECK_ERROR;
}

void Geometry::draw(const string &drawStyle) {

	if (drawStyles.find(drawStyle) == drawStyles.end()) {
		throw;
	}

	DrawCall *call = drawStyles[drawStyle];
	call->activate();

	GL_CHECK_ERROR;

	setUniforms(call->shaderProgram());

	GL_CHECK_ERROR;

	startStream(call);

	call->deactivate();
	cleanUpGL();

	GL_CHECK_ERROR;
}

void Geometry::startStream(DrawCall *call) {

	UNREFERENCED_PARAMETER(call);

	glDrawArrays(primitiveType, primitiveStart, numPrimitives);

	GL_CHECK_ERROR;
}

void Geometry::cleanUpGL() {

	for (int i = 0; i < numActiveTextures; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}

	GL_CHECK_ERROR;
}

