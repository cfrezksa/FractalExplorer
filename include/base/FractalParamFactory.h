#pragma once
#include <iostream>
#include <map>
#include <string>
#include "FractalParamT.h"
#include "../dll/FractalLib.h"

namespace Fractals {

	class FRACTALLIB_API FractalParamFactory {
	public:
		static int registerParam(const std::string &name, IParamFactory *);
		static IFractalParam *read(const std::string & line);

	private:

#pragma warning( push )
#pragma warning( disable : 4251)
		static std::map <std::string, IParamFactory *> factoryMap;
#pragma warning( pop )

		static int count;
	};
}
