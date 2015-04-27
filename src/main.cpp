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
	cLogRotate lr("test.conf");

	std::this_thread::sleep_for(std::chrono::seconds(3));
	lr.run();

	std::this_thread::sleep_for(std::chrono::seconds(3));

	return 0;
}

