/*
 * main.cpp
 *
 *  Created on: Apr 23, 2015
 *      Author: robert
 */




#include <iostream>
#include "cLogRotate.h"

int main()
{
	cLogRotate log_rotate(10, 10);
	auto vect = log_rotate.getFileVector(std::string(R"(\./test\.log\.\d+)"));
	for (auto a : vect)
	{
		std::cout << a << std::endl;
	}
	return 0;
}

