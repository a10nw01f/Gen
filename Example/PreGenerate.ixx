module;
#include <array>
#include <string>
#include <source_location>

/*INSERT_POST_GENERATE_MACRO*/
//#define POST_GENERATE

#ifndef POST_GENERATE
#define PRE_GENERATE
#define MODULE_NAME PreGenerate
#else
#define MODULE_NAME PostGenerate
#endif

export module MODULE_NAME;

#ifndef PRE_GENERATE

import PreGenerate;

export class C 
{
public:
	/*INSERT_FUNC*/

};

#else

import Gen.Inject;

namespace
{
	static void Generate()
	{
		// the injection happens when this function call is being compiled
		Gen::Inject<
			{"PreGenerate.ixx"}, {"PostGenerate.ixx"},
			"/*INSERT_POST_GENERATE_MACRO*/", "#define POST_GENERATE",
			"/*INSERT_FUNC*/", "void Foo(){}">();
	}
}

#endif