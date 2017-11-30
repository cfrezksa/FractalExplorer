#pragma once

#define NOMINMAX
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "../dll/FractalLib.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <float.h>
#include <assert.h>
#include <iterator>

namespace Fractals {
	
	class FRACTALLIB_API IFractalParam{
	public:
		virtual std::string typeName() const = 0;
		virtual const std::string &paramName() const = 0;
		virtual const void *value() const = 0;
		virtual void setValue(int i, void *p) = 0;
		virtual size_t size() const = 0;
		virtual size_t componentCount() const = 0;
	};

	class FRACTALLIB_API IParamFactory {
	public:
		virtual IFractalParam *create(const std::string &data) = 0;
	};

	template<typename T, size_t N> class  FractalParamT : public IFractalParam
	{
	public:
		class Factory : public IParamFactory {
		public:
			IFractalParam *create(const std::string &data) {
				FractalParamT *p = new FractalParamT();
				p->read(data);
				return p;
			}

		};
	public:
		FractalParamT();
		FractalParamT(const std::string &name,
			const std::vector<T> values,
			const std::vector<T> minValues,
			const std::vector<T> maxValues);

		FractalParamT(const std::string &name,
			const std::vector<T> values);

		virtual std::string typeName() const {
			std::string result = typeid(T).name();
			result += std::to_string(N);
			return result;
		}
		
		virtual const std::string &paramName() const {
			return name;
		}
		
		virtual size_t componentCount() const {
			return N;
		}

		const void * value() const { return (void *)& paramValues[0]; }

		size_t size() const { return N * sizeof(T); }

		void setValue(int i, void *p) {
			assert(i < N);
			T* v = (T*)p;
			paramValues[i] = *v;
		};

		void write(std::ostream &stream) const;

		void read(const std::string &data);

	private:
		std::string name;
		T paramValues[N];
		T paramMinValues[N];
		T paramMaxValues[N];
	public:
		static const int registrationId;

		friend 
		std::ostream & operator<<(std::ostream & os, const FractalParamT& p)
		{
			p.write(os);
			return os;
		}
	};

	template<class T, size_t N> FractalParamT<T, N>::FractalParamT(const std::string &n,
		const std::vector<T> values,
		const std::vector<T> minValues,
		const std::vector<T> maxValues) {

		name = n;

		assert(values.size() == N);
		assert(minValues.size() == N);
		assert(maxValues.size() == N);

		for (size_t i = 0; i < N; i++) {
			paramValues[i] = values[i];
			paramMinValues[i] = minValues[i];
			paramMaxValues[i] = maxValues[i];
		}

	}


	template<class T, size_t N> FractalParamT<T, N>::FractalParamT() {

		name = "";

		for (size_t i = 0; i < N; i++) {
			paramValues[i] = (T) 0;
			paramMinValues[i] =  numeric_limits<T>::min();
			paramMaxValues[i] =  numeric_limits<T>::max();
		}

	}


	template<typename T, size_t N>  FractalParamT<T, N>::FractalParamT(const std::string &n,
		const std::vector<T> values) {

		name = n;

		assert(values.size() == N);

		for (size_t i = 0; i < N; i++) {
			paramValues[i] = values[i];
			paramMinValues[i] = numeric_limits<T>::min();
			paramMaxValues[i] = numeric_limits<T>::max();
		}

	}

	 template<typename T, size_t N> void FractalParamT<T, N>::write(std::ostream &stream) const {
		stream << typeid(T).name() << N << " ";
		stream << name << " ";
		for (size_t i = 0; i < N; i++) {
			stream << paramValues[i] << " ";
		}
	}

	template<typename T, size_t N> void FractalParamT<T, N>::read(const std::string &data) {
		
		istringstream iss(data);
		vector<string> tokens{ istream_iterator<string>{iss},
			istream_iterator<string>{} };

		int numTokensRead = 0;

		string tname = tokens[numTokensRead++];
		assert(tname == typeName());

		name = tokens[numTokensRead++];
		
		if (tokens.size() - numTokensRead > N) {
			for (size_t i = 0; i < N; i++) {
				istringstream dataS(tokens[numTokensRead++]);
				dataS >> paramMinValues[i];
			}
		}

		if (tokens.size() - numTokensRead > N) {
			for (size_t i = 0; i < N; i++) {
				istringstream dataS(tokens[numTokensRead++]);
				dataS >> paramMinValues[i];
			}
		}

		if (tokens.size() - numTokensRead > N) {
			for (size_t i = 0; i < N; i++) {
				istringstream dataS(tokens[numTokensRead++]);
				dataS >> paramMinValues[i];
			}
		}
	}

	template<typename T, size_t N> const int FractalParamT<T, N>::registrationId = FractalParamFactory::registerParam(
		std::string(typeid(T).name()) + std::to_string(N), new FractalParamT<T, N>::Factory());

#pragma warning( push )
#pragma warning( disable : 4251)

	class FRACTALLIB_API ParamFloat : public FractalParamT<float, 1> {};
	class FRACTALLIB_API ParamVector2 : public FractalParamT<float, 2> {};
	class FRACTALLIB_API ParamVector3 : public FractalParamT<float, 3> {};
	class FRACTALLIB_API ParamVector4 : public FractalParamT<float, 4> {};
	class FRACTALLIB_API ParamMat3 : public FractalParamT<float, 9> {};
	class FRACTALLIB_API ParamMat4 : public FractalParamT<float, 16> {};
	class FRACTALLIB_API ParamInt : public FractalParamT<int, 1> {};
	class FRACTALLIB_API ParamBool : public FractalParamT<bool, 1> {};

#pragma warning( pop ) 
}
