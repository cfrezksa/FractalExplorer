#pragma once
#include<vector>
#include <string>
#include <sstream>
#include <iterator>

namespace InlineUtilities {

	using namespace std;

	inline vector<string> split(const string &line) {

		istringstream iss(line);

		vector<string> tokens{
			istream_iterator<string>{iss},
			istream_iterator<string>{} };

		return tokens;
	}

}