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
	std::vector<std::string> getFileVector(const boost::regex &fileRegex); // TODO private
private:
	std::vector<std::string> mFileVector;
	unsigned int mMaxLogFiles;
	unsigned int mMaxGZFiles;
	boost::uintmax_t mMinDiscFreeSpace;
	fs::path mPath;
	std::chrono::hours mMaxLogStorageTime;
	const boost::regex mFileRegex; // (...)test.log.1, (...)test.log.2, ... , (...)test.log.n
	const boost::regex mGZFileRegex; // (...)test.log.1.gz, (...)test.log.2.gz, ... , (...)test.log.n.gz

	boost::uintmax_t getFreeSpace();
	std::string getSuffix(const std::string &str);
	//std::string getPrefix(const std::string &str);

	struct sFileName
	{
		std::string prefix;
		std::string suffix;
	};

	sFileName convertName(const std::string &fileName);
};

#endif /* CLOGROTATE_H_ */
