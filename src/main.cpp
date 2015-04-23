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
	cLogRotate log_rotate(10, 10, 1000, "/home/robert/log_rotate/");
	//log_rotate.setFileRegexName(std::string(R"(/home/robert/log_rotate/test\.log\.\d+)"));
	//log_rotate.rotate();
	auto vect = log_rotate.getFileVector();
	for	 (auto a : vect)
	{
		std::cout << a << std::endl;
	}
	return 0;
}

