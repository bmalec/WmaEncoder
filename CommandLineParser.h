#pragma once

#include "Parameters.h"




class CommandLineParser
{

public:
	static void Parse(int argc, WCHAR* argv[], Parameters* parameters);
};
