#include "fractals\Fractal.h"
#include <fstream>
#include <sstream>
#include <iterator>

VertexBuffer *Fractal::staticVertexBuffer = NULL;

Fractal::Fractal(const string &filename) {
	commonConstructor();
	read(filename);
}

Fractal::Fractal() {
	commonConstructor();
}

void Fractal::commonConstructor() {
	if (NULL == staticVertexBuffer) {
		init();
	}
	vertexBuffer = staticVertexBuffer;
	numPrimitives = 4;
	primitiveStart = 0;
	stride = numFloatsPerVertex * sizeof(float);
	primitiveType = GL_TRIANGLE_FAN;
	addBufferOffset("position", 0);

}

void Fractal::init() {

	typeTokens.push_back(string("float"));
	typeTokens.push_back(string("int"));
	typeTokens.push_back(string("bool"));
	typeTokens.push_back(string("vector2"));
	typeTokens.push_back(string("vector3"));
	typeTokens.push_back(string("vector4"));
	typeTokens.push_back(string("matrix3"));
	typeTokens.push_back(string("matrix4"));

	int numVertices = 4;
	numFloatsPerVertex = 3;
	int numFloatsTotal = numVertices * numFloatsPerVertex;

	float *vertices = new float[numFloatsTotal];

	int vidx = 0;
	vertices[vidx++] = -1.0f; vertices[vidx++] =  1.0f; vertices[vidx++] = 0.0f; // 0
	vertices[vidx++] =  1.0f; vertices[vidx++] =  1.0f; vertices[vidx++] = 0.0f; // 1
	vertices[vidx++] =  1.0f; vertices[vidx++] = -1.0f; vertices[vidx++] = 0.0f; // 2
	vertices[vidx++] = -1.0f; vertices[vidx++] = -1.0f; vertices[vidx++] = 0.0f; // 3

	assert(vidx == numFloatsTotal);

	staticVertexBuffer = new VertexBuffer(vertices, (numFloatsTotal) * sizeof(float));;
	

}

Fractal::~Fractal() {

}

const int Fractal::NUM_TYPE_TOKENS = 10;
vector<string> Fractal::typeTokens;
int Fractal::numFloatsPerVertex;
map<string, ShaderProgram*> Fractal::shaders;

int Fractal::TypeTokenId(const string &token) {

	for (int i = 0; i < (int) typeTokens.size(); i++) {
		if (typeTokens[i] == token) return i;
	}
	return -1;
}

void Fractal::read(const string &filename) {
	
	ifstream dataFile;
	dataFile.open(filename);      // open input file
	
	assert(dataFile.is_open());

	string identifier;
	getline(dataFile, identifier);
	if (identifier != "Fractal Explorer") throw;

	string shader;
	getline(dataFile, shader);
	setType(shader);

	string line;
	while (getline(dataFile,line)) {
		if ((line.empty()) || (line[0] == '/')) continue;

		istringstream iss(line);
		vector<string> tokens;
		copy(istream_iterator<string>(iss),
			istream_iterator<string>(),
			back_inserter(tokens));

		if (tokens.size() < 3) continue;

		string paramName = tokens[0];
		string type = tokens[1];
		int idx = 2;

		Matrix3f matrix3x3;
		Matrix4f matrix4x4;

		std::string pname = tokens[0];

		switch (TypeTokenId(type)) {
		case 0: // "float",
			assert(tokens.size() == 3);
			setFloat(paramName, stof(tokens[2]));
			break;
		case 1: // "int",
			assert(tokens.size() == 3);
			setInt(paramName, stoi(tokens[2]));
			break;
		case 2: // "bool",
			assert(tokens.size() == 3);
			setBool(paramName, stoi(tokens[2]) != 0);
			break;
		case 3: // "vector2",
			assert(tokens.size() == 4);
			setVector(paramName, Vector2f(stof(tokens[2]), stof(tokens[3])));
			break;
		case 4: // "vector3",
			assert(tokens.size() == 5);
			setVector(paramName, Vector3f(stof(tokens[2]), stof(tokens[3]), stof(tokens[4])));
			break;
		case 5: // "vector4",
			assert(tokens.size() == 6);
			setVector(paramName, Vector4f(stof(tokens[2]), stof(tokens[3]), stof(tokens[4]), stof(tokens[5])));
			break;
		case 6: // "matrix3",
			assert(tokens.size() == 11);

			for (int r = 0; r < 3; r++) {
				for (int c = 0; c < 3; c++) {
					matrix3x3(r, c) = stof(tokens[idx++]);
				}
			}
			setMatrix(paramName, matrix3x3);
			break;
		case 7: // "matrix4",
			assert(tokens.size() == 18);

			for (int r = 0; r < 4; r++) {
				for (int c = 0; c < 4; c++) {
					matrix4x4(r, c) = stof(tokens[idx++]);
				}
			}
			setMatrix(paramName, matrix4x4);
			break;
		}
	}
}

void Fractal::write(const string &filename) {

	ofstream dataFile;
	dataFile.open(filename);

	assert(dataFile.is_open());

	dataFile << "Fractal Explorer\n";
	dataFile << drawStyle << "\n";

	for each (auto entry in floatParams)
	{
		dataFile << entry.first << " float " << entry.second << "\n";
	}
	
	for each (auto entry in intParams)
	{
		dataFile << entry.first << " float " << entry.second << "\n";
	}
	
	for each (auto entry in boolParams)
	{
		dataFile << entry.first << " float " << entry.second << "\n";
	}
	
	for each (auto entry in vec2Params) {
		Vector2f vec = entry.second;
		dataFile << entry.first << " vector2 " << vec[0] << " " << vec[1] << "\n";
	}

	for each (auto entry in vec3Params) {
		Vector3f vec = entry.second;
		dataFile << entry.first << " vector3 " << vec[0] << " " << vec[1] << " " << vec[2] << "\n";
	}

	for each (auto entry in vec4Params) {
		Vector4f vec = entry.second;
		dataFile << entry.first << " vector4 " << vec[0] << " " << vec[1] << " " << vec[2] << " " << vec[3] << "\n";
	}


	for each (auto entry in mat3x3Params) {
		Matrix3f mat = entry.second;
		dataFile << entry.first << " matrix3 ";
		for (int r = 0; r < 3; r++) {
			for (int c = 0; c < 3; c++) {
				dataFile << mat(r, c) << " ";
			}
		}
	}

	for each (auto entry in mat4x4Params) {
		Matrix4f mat = entry.second;
		dataFile << entry.first << " matrix4 ";
		for (int r = 0; r < 4; r++) {
			for (int c = 0; c < 4; c++) {
				dataFile << mat(r, c) << " ";
			}
		}
	}
}

void Fractal::draw() {
	Geometry::draw(drawStyle);
}

void Fractal::setType(const string &type) {

	drawStyle = type;

	if (drawStyles.find(type) == drawStyles.end()) {
		ShaderProgram *shader = new ShaderProgram("../shaders/standardVP.glsl", "../shaders/" + type + ".glsl");
		shader->create();
		addDrawStyle(type, shader);
	}
}