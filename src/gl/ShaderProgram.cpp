#include "core/globalDefs.h"
#include "gl/ShaderProgram.h"
#include <fstream>


char *ShaderProgram::infoLog = NULL;
int   ShaderProgram::infoLogLength = 0;

GLchar *readShaderFile(const string &filename) {
	std::ifstream textStream;
	int length;
	textStream.open(filename, std::ios::binary);      // open input file
	assert(textStream.is_open());
	textStream.seekg(0, std::ios::end);    // go to the end
	length = (int)textStream.tellg();           // report location (this is the length)
	textStream.seekg(0, std::ios::beg);    // go back to the beginning
	GLchar *buffer = new GLchar[length + 1];    // allocate memory for a buffer of appropriate dimension
	textStream.read(buffer, length);       // read the whole file into the buffer
	textStream.close();                    // close file handle
	buffer[length] = '\0';
	return buffer;
}


ShaderProgram::ShaderProgram(const string &vs, const string& fs) {
	progId = 0;
	vertexShaderFile = vs;
	fragmentShaderFile = fs;
}

ShaderProgram::~ShaderProgram() { 
	if (0 != progId) {
		glDeleteProgram(progId);
	}

}
void ShaderProgram::create() { 

	const GLchar *vertexSource = readShaderFile(vertexShaderFile);
	const GLchar *fragmentSource = readShaderFile(fragmentShaderFile);

	GL_CHECK_ERROR;

	/* Assign our handles a "name" to new shader objects */
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	assert(vertexShader != 0);
	assert(fragmentShader != 0);

	/* Associate the source code buffers with each handle */
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	GL_CHECK_ERROR;
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	GL_CHECK_ERROR;

	delete[] vertexSource;
	delete[] fragmentSource;

	int compiled;
	/* Compile our shader objects */
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
	if (compiled == false) {
		checkShaderLog(vertexShader);
		throw;
	}

	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
	if (compiled == false) {
		checkShaderLog(fragmentShader);
		throw "GLSL: Unable to Compile: Error in Fragment Shader";
	}

	progId = glCreateProgram();

	GL_CHECK_ERROR;
	/* Attach our shaders to our program */
	glAttachShader(progId, vertexShader);
	glAttachShader(progId, fragmentShader);

	glLinkProgram(progId);
	int linked;

	glGetProgramiv(progId, GL_LINK_STATUS, &linked);
	if (linked == false) {
		checkProgramLog(progId);
		throw "GLSL: Unable to Link: Error in Shader";
	}

	GLint numUniforms = -1;
	glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &numUniforms);

	glUseProgram(0);

}

void  ShaderProgram::checkLogBuffers(int length) {
	if (length > infoLogLength) {
		if (infoLog) delete[] infoLog;
		infoLogLength = length;
		infoLog = new char[infoLogLength];
	}
}

bool ShaderProgram::checkShaderLog(GLuint shader) {

	int infoLen;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
	checkLogBuffers(infoLen);

	if (infoLen > 0) {
		glGetShaderInfoLog(shader, infoLogLength, &infoLen, infoLog);
		OVR_DEBUG_LOG(">> InfoLog: ");
		OVR_DEBUG_LOG(infoLog);
		OVR_DEBUG_LOG("end of InfoLog: ");
		return false;
	}
	return true;

}

bool ShaderProgram::checkProgramLog(GLuint shader) {

	int infoLen;
	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
	checkLogBuffers(infoLen);

	if (infoLen > 0) {
		glGetProgramInfoLog(shader, infoLogLength, &infoLen, infoLog);
		OVR_DEBUG_LOG(">> InfoLog: ");
		OVR_DEBUG_LOG(infoLog);
		OVR_DEBUG_LOG("end of InfoLog: ");
		return false;
	}
	return true;

}

void ShaderProgram::activate() {
	assert(progId != 0);
	glUseProgram(progId);
}

void ShaderProgram::deactivate() {
	glUseProgram(0);
}

int  ShaderProgram::attributeLocation(const string &attribName) {

	int result = glGetAttribLocation(progId, attribName.c_str());
	return result;
	
}

void ShaderProgram::enableAttributeArray(int varLoc) { 
	glEnableVertexAttribArray(varLoc);
	GL_CHECK_ERROR;
}

void ShaderProgram::setAttributeArray(int varLoc, GLenum type, const void* offset, int tupleSize, int stride) { 

	glVertexAttribPointer(varLoc, tupleSize, type, GL_FALSE, stride, offset);

}

int  ShaderProgram::uniformLocation(const string &name) {
	return glGetUniformLocation(progId, name.c_str());
}

void ShaderProgram::setUniformValue(int uniLoc, GLuint texActive) {
	glUniform1i(uniLoc, texActive);
}

void ShaderProgram::setUniformValue(int uniLoc, float value) {
	glUniform1f(uniLoc, value);
}

void ShaderProgram::setUniformValue(int uniLoc, Vector2f value) { 
	glUniform2fv(uniLoc, 1, (float *) &value);
}

void ShaderProgram::setUniformValue(int uniLoc, Vector3f value) {
	glUniform3fv(uniLoc, 1, (float *)&value);

}

void ShaderProgram::setUniformValue(int uniLoc, Vector4f value) {
	glUniform4fv(uniLoc, 1, (float *)&value);
}

void ShaderProgram::setUniformValue(int uniLoc, Matrix4f value) {
	glUniformMatrix4fv(uniLoc, 1, GL_TRUE, (FLOAT*)&value);

}

void ShaderProgram::setUniformValue(int uniLoc, Matrix3f value) { 
	glUniformMatrix3fv(uniLoc, 1, GL_TRUE, (FLOAT*)&value);
}

void ShaderProgram::setUniformValue(int uniLoc, int value) { 
	glUniform1i(uniLoc, value);
}

void ShaderProgram::setUniformValue(int uniLoc, bool value) { 
	glUniform1i(uniLoc, value);
}
