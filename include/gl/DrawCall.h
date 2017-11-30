#pragma once

class ShaderProgram;
class VertexArrayObject;

class DrawCall {

public:
	DrawCall();
	~DrawCall();
	void activate();
	void deactivate();
	ShaderProgram *shaderProgram() { return shader; }
	void prepare(ShaderProgram *shader);
protected:

	VertexArrayObject *vao;
	ShaderProgram *shader;

};