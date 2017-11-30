#pragma once
#include <string>
#include <vector>
#include "FractalParamT.h"
#include "../dll/FractalLib.h"

namespace Fractals {

	class FRACTALLIB_API FractalShader
	{
	public:
		FractalShader(const std::string &filename);
		bool searchFor(std::ifstream &stream, const std::string &toWaitFor);
		virtual ~FractalShader();

		void write(const std::string &filename);
		void read(const std::string &filename);
		std::string fragmentShader();
#pragma warning( push )
#pragma warning( disable : 4251)
	private:
		std::vector < std::string > snippets;
#pragma warning( pop ) 
	};

}
