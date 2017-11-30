#pragma once
#include "base/FractalParamFactory.h"
#include "base/InlineUtilities.h"
#include <sstream>
#include <iterator>

using namespace Fractals;
using namespace std;


int FractalParamFactory::registerParam(const std::string &name, IParamFactory *factory) {

	factoryMap[name] = factory;
	return ++count;

}

IFractalParam *FractalParamFactory::read(const std::string &line) {
	
	vector<string> tokens = InlineUtilities::split(line);
		
	string typeName = tokens[0];
	
	IFractalParam *param = NULL;

	auto it = factoryMap.find(typeName);

	if (it != factoryMap.end()) {
		IParamFactory *factory = (*it).second;
		param = factory->create(line);
	}

	return param;
}

std::map <std::string, IParamFactory *> FractalParamFactory::factoryMap;
int FractalParamFactory::count = 0;
