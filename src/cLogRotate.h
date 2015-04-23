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
	cLogRotate(boost::uintmax_t minDiscFreeSpace, std::string path = std::string("."));
	virtual ~cLogRotate();

	void rotate();
	std::vector<std::string> getFileVector(const std::string &regex_str);
private:
	fs::path mPath;
	std::vector<std::string> mFileVector;
	boost::uintmax_t mMinDiscFreeSpace;
	unsigned int mMaxLogFiles;
	unsigned int mMaxGZFiles;
	boost::uintmax_t getFreeSpace();
	//std::chrono::duration<int, std::chrono::hours> mMaxLogStorageTime;
};

#endif /* CLOGROTATE_H_ */
