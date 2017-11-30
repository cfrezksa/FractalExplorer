#pragma once
#include "IGeometry.h"
#include "../GLE/CAPI_GLE.h"
#include "../gl/TextureBuffer.h"

#include <map>
#include <vector>
#include "../gl/GLBuffers.h"

class DrawCall;

class  Geometry : public IGeometry
{

public:
	Geometry();
	Geometry(VertexBuffer *vb, GLenum primType, GLint primStart, GLsizei primCount, GLint stride);
	virtual ~Geometry();
	virtual void addDrawStyle(const string &name, ShaderProgram *shader);
	virtual void draw(const string &drawStyle);
	virtual void addBufferOffset(const string &name, GLint offset) { bufferOffsets[name] = offset; }

	void setTexture(const string &name, unsigned int id) { textures[name] = id; }
	void setTexture(const string &name, TextureBuffer *tex) { textures[name] = tex->textureId(); }
	void setInt(const string &name, int i) { intParams[name] = i; }
	void setBool(const string &name, bool i) { boolParams[name] = i; }
	void setFloat(const string &name, float f) { floatParams[name] = f; }
	void setVector(const string &name, Vector2f v) { vec2Params[name] = v; };
	void setVector(const string &name, Vector3f v) { vec3Params[name] = v; };
	void setVector(const string &name, Vector4f v) { vec4Params[name] = v; };
	void setMatrix(const string &name, Matrix4f mat) { mat4x4Params[name] = mat; }
	void setMatrix(const string &name, Matrix3f mat) { mat3x3Params[name] = mat; }

protected:
	string meshFile;
	VertexBuffer *vertexBuffer;

	int numPrimitives;
	int primitiveStart;
	int numActiveTextures;
	int stride;
	GLenum primitiveType;

	map<string, unsigned int> textures;
	map<string, vector<unsigned int>> textureArrays;
	map<string, float> floatParams;
	map<string, int> intParams;
	map<string, bool> boolParams;
	map<string, Vector2f> vec2Params;
	map<string, Vector3f> vec3Params;
	map<string, Vector4f> vec4Params;
	map<string, Matrix4f> mat4x4Params;
	map<string, Matrix3f> mat3x3Params;
	map<string, DrawCall*> drawStyles;
	map<string, unsigned int> bufferOffsets;

	unsigned int setTexParams(ShaderProgram *shader);
	void setUniforms(ShaderProgram *shader);

	virtual void prepareDrawStyle(DrawCall *call);
	virtual void cleanUpGL();
	virtual void startStream(DrawCall *call);
	virtual void bindGeometry();

	friend class DrawCall;
};

