export module Gen.System;

import Gen.Utils;
using namespace Gen;

export namespace Gen
{
	export template<StaticString command>
	constexpr void System()
	{
		StaticPrint<StaticString("System").Array(), command.Array()>::Print();
	};
}