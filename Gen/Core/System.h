#pragma once
#include "std.h"
#include "StaticPrint.h"
#include "StaticString.h"

namespace Gen
{
	template<StaticString command>
	constexpr void System()
	{
		StaticPrint<StaticString("System").Array(), command.Array()>::Print();
	}
}