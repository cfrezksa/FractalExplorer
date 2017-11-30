#include "base/FractalShader.h"
#include "base/FractalParamT.h"
#include "base/FractalParamFactory.h"
#include <fstream>
#include <strstream>
#include <algorithm>

using namespace Fractals;
using namespace std;

static char cLine[512];

FractalShader::FractalShader(const std::string &filename)
{
	read(filename);
}

void FractalShader::read(const std::string &filename)
{
	ifstream stream;
	stream.open(filename);
	assert(stream.is_open());

	stream.getline(&cLine[0], 512);
	std::string header = cLine;

	assert(header == "FRACTAL SHADER ASSEMBLY");

	if (!searchFor(stream, "begin assembly")) {
		throw;
	}

	string endToken = "end assembly";

	while (stream.is_open()) {

		stream.getline(&cLine[0], 512);
		std::string line = cLine;

		if (line.substr(0, endToken.size()) == endToken) {
			break;
		}
		line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
		snippets.push_back(line);

	}

}


string readFile(const string &filename) {
	std::ifstream textStream;
	int length;
	textStream.open(filename, std::ios::binary);      // open input file
	assert(textStream.is_open());
	textStream.seekg(0, std::ios::end);    // go to the end
	length = (int)textStream.tellg();           // report location (this is the length)
	textStream.seekg(0, std::ios::beg);    // go back to the beginning
	char *buffer = new char[length + 1];    // allocate memory for a buffer of appropriate dimension
	textStream.read(buffer, length);       // read the whole file into the buffer
	textStream.close();                    // close file handle
	buffer[length] = '\0';
	string s = buffer;
	delete[] buffer;
	return s;
}



string FractalShader::fragmentShader() {

	string shader = "";
		
	ifstream snippetStream;

	for each (auto snippet in snippets)
	{
		shader += readFile("../shaders/snippets/" + snippet + ".snippet");
	}

	return shader;

}

	//int id = ParamFloat::registrationId;
	//id = ParamVector2::registrationId;
	//id = ParamVector3::registrationId;
	//id = ParamVector4::registrationId;
	//
	//std::vector<float> values;
	//values.push_back(0.0f);
	//ParamFloat p0("param0", values);
	//
	//values.push_back(1.0f);
	//ParamVector2 p1("param1", values);
	//
	//values.push_back(2.0f);
	//ParamVector3 p2("param2", values);
	//
	//values.push_back(3.0f);
	//ParamVector4 p3("param3", values);
	//
	//std::ofstream outstream;
	//outstream.open("Test.params");
	//
	//outstream << p0 << "\n";
	//outstream << p1 << "\n";
	//outstream << p2 << "\n";
	//outstream << p3 << "\n";
	//
	//outstream.close();
	//
	//std::ifstream instream;
	//instream.open("Test.params");
	//
	//IFractalParam *q0 = FractalParamFactory::read(instream);
	//assert(NULL != q0);
	//IFractalParam *q1 = FractalParamFactory::read(instream);
	//assert(NULL != q1);
	//IFractalParam *q2 = FractalParamFactory::read(instream);
	//assert(NULL != q2);
	//IFractalParam *q3 = FractalParamFactory::read(instream);
	//assert(NULL != q3);
	//



bool Fractals::FractalShader::searchFor(std::ifstream &stream, const std::string &toWaitFor)
{
	while (stream.is_open()) {
		stream.getline(&cLine[0], 512);
		std::string line = cLine;

		if (line.substr(0, toWaitFor.size()) == toWaitFor) {
			return true;
		}
	}
	return false;
}

FractalShader::~FractalShader()
{

}
