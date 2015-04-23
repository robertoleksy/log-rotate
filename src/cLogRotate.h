/*
 * cLogRotate.h
 *
 *  Created on: Apr 20, 2015
 *      Author: robert
 */

#ifndef CLOGROTATE_H_
#define CLOGROTATE_H_

#include <vector>
#include <string>
#include <chrono>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp> // http://stackoverflow.com/questions/13899276/using-regex-under-c11

namespace fs = boost::filesystem;

class cLogRotate {
public:
	cLogRotate(unsigned int maxLogFiles, unsigned int maxGZFiles, boost::uintmax_t minDiscFreeSpace = 1000,
			std::string path = std::string("."), std::chrono::hours maxLogStorageTime = std::chrono::hours(24 * 30));
	virtual ~cLogRotate();

	void rotate();
	std::vector<std::string> getFileVector(const std::string &regex_str);
private:
	std::vector<std::string> mFileVector;
	unsigned int mMaxLogFiles;
	unsigned int mMaxGZFiles;
	boost::uintmax_t mMinDiscFreeSpace;
	fs::path mPath;
	boost::uintmax_t getFreeSpace();
	std::chrono::hours mMaxLogStorageTime;
};

#endif /* CLOGROTATE_H_ */
