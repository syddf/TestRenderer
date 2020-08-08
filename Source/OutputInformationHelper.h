#pragma once
#include "Prefix.h"
#include <iostream>
#include <fstream>

class OutputInformationHelper
{
public:
	static void OutputErrorInfomation(const std::string info)
	{
		static std::ofstream OutputStream;
		std::string outputLine = "[Error]" + info + "\n";
		if (!OutputStream)
		{
			OutputStream.open("Error.txt", std::ios::app | std::ios::out);
		}
		if (OutputStream)
		{
			OutputStream << outputLine;
		}
		std::cout << outputLine;
	}
};