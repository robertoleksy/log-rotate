/*
 * cLogRotate.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: robert
 */

#include "cLogRotate.h"

cLogRotate::cLogRotate(unsigned int maxLogFiles, unsigned int maxGZFiles, boost::uintmax_t minDiscFreeSpace, std::string path, std::chrono::hours maxLogStorageTime)
:
  mMaxLogFiles(maxLogFiles),
  mMaxGZFiles (maxGZFiles),
  mMinDiscFreeSpace (minDiscFreeSpace),
  mPath (path),
  mMaxLogStorageTime (maxLogStorageTime)
{
}

cLogRotate::~cLogRotate()
{
}

boost::uintmax_t cLogRotate::getFreeSpace()
{
	fs::space_info space_inf = fs::space(mPath);
	return space_inf.available;
}

std::vector<std::string> cLogRotate::getFileVector(const std::string &regex_str)
{
	boost::regex regex_file(regex_str);
	fs::directory_iterator end_iter;
	std::string fileName;
	std::vector<std::string> fileVector;

	for( fs::directory_iterator dir_iter(mPath) ; dir_iter != end_iter ; ++dir_iter)
	{
		if (fs::is_regular_file(dir_iter->status()))
		{
			fileName = dir_iter->path().c_str();
			if (boost::regex_match(fileName, regex_file))
			{
				fileVector.emplace_back(std::move(fileName));
			}
		}
	}

	return fileVector;
}
