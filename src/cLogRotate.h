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
#include <fstream>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp> // http://stackoverflow.com/questions/13899276/using-regex-under-c11

namespace fs = boost::filesystem;

class cLogRotate {
public:
	cLogRotate();
	cLogRotate(const std::string &confFileName);
	virtual ~cLogRotate();

	void rotate();
private:
	std::vector<std::string> getFileVector(const boost::regex &fileRegex);
	std::vector<std::string> mFileVector;
	unsigned int mMaxLogFiles;
	unsigned int mMaxGZFiles;
	boost::uintmax_t mMinDiscFreeSpace;
	boost::uintmax_t mMaxLogsSize;
	fs::path mPath;
	std::chrono::hours mMaxLogStorageTime;
	std::chrono::seconds mSleepTime;
	std::string mInstance;
	boost::regex mFileRegex;
	boost::regex mGZFileRegex;
	const unsigned int sizeOfFileType = 3; // std::string(".gz").size();
	boost::uintmax_t getFreeSpace();
	std::string getSuffix(const std::string &str);
	static const std::string mLogFileBaseRegex;
	static const std::string mGZFileBaseRegex;
	std::ifstream mConfigFile;

	struct sFileName
	{
		std::string prefix;
		std::string suffix; // last number in path without ".gz"
	};

	sFileName convertName(const std::string &fileName);
	std::chrono::system_clock::time_point lastWriteTime(const std::string &path);
	std::string getNextValueFromFile();
	bool parseConfFile();
};

#endif /* CLOGROTATE_H_ */
