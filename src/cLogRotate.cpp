/*
 * cLogRotate.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: robert
 */

#include "cLogRotate.h"

cLogRotate::cLogRotate(boost::uintmax_t minDiscFreeSpace, std::string path) {
	// TODO Auto-generated constructor stub

}

cLogRotate::~cLogRotate() {
	// TODO Auto-generated destructor stub
}

boost::uintmax_t cLogRotate::getFreeSpace()
{
	fs::space_info space_inf = fs::space(boost::filesystem::path("/home"));
	return space_inf.available;
}

std::vector<std::string> cLogRotate::getFileVector(const std::string &regex_str)
{
	boost::regex regex_file(regex_str);
	fs::directory_iterator end_iter;
	std::string fileName;
	std::vector<std::string> fileVector;
	for (fs::directory_iterator dir_iter(mPath); dir_iter != end_iter; ++dir_iter)
	{
		if (fs::is_regular_file(dir_iter->status()))
		{
			fileName = dir_iter->path().c_str();
			if (boost::regex_match(regex_str, regex_file))
			{
				fileVector.emplace_back(fileName);
			}
		}
	}
	return fileVector;
}
