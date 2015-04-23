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
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp> // http://stackoverflow.com/questions/13899276/using-regex-under-c11

namespace fs = boost::filesystem;

class cLogRotate {
public:
	cLogRotate(unsigned int maxLogFiles, unsigned int maxGZFiles, boost::uintmax_t minDiscFreeSpace = 1000,
			std::string path = std::string("."), std::chrono::hours maxLogStorageTime = std::chrono::hours(24 * 30));
	virtual ~cLogRotate();

	void rotate();
	void setFileRegexName(const std::string &regexName);
	std::vector<std::string> getFileVector(const std::string &regex_str); // TODO private
private:
	std::vector<std::string> mFileVector;
	unsigned int mMaxLogFiles;
	unsigned int mMaxGZFiles;
	boost::uintmax_t mMinDiscFreeSpace;
	fs::path mPath;
	std::chrono::hours mMaxLogStorageTime;
	std::string mRegexFileName; // http://www.boost.org/doc/libs/1_58_0/libs/regex/doc/html/boost_regex/syntax/basic_extended.html

	boost::uintmax_t getFreeSpace();
};

#endif /* CLOGROTATE_H_ */
