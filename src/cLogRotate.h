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
#include <atomic>
#include <mutex>
#include <cctype>
#include <memory>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp> // http://stackoverflow.com/questions/13899276/using-regex-under-c11
#include <boost/date_time.hpp>
#include <zlib.h>

namespace fs = boost::filesystem;

class cLogRotate {
public:
	cLogRotate(const std::string &confFileName);
	virtual ~cLogRotate();

	void run();
	//std::mutex mLogSaveMutex; // TODO
private:
	std::vector<std::string> getFileVector(const boost::regex &fileRegex); // TODO return std::set?
	unsigned int mMaxLogFiles;
	unsigned int mMaxGZFiles;
	boost::uintmax_t mMinDiscFreeSpace;
	boost::uintmax_t mMaxLogSize; // one file
	boost::uintmax_t mMaxAllLogsSize; // all files (normal + .gz)
	fs::path mPath;
	std::chrono::seconds mMaxLogStorageTime;
	std::chrono::seconds mSleepTime;
	std::string mInstance;
	boost::regex mFileRegex;
	boost::regex mGZFileRegex;
	const unsigned int sizeOfFileType = 3; // std::string(".gz").size();
	static const std::string mLogFileBaseRegex;
	static const std::string mGZFileBaseRegex;
	std::ifstream mConfigFile;
	unsigned int mSingleLines; // max lines in single file
	std::chrono::seconds mSingleTime;
	std::unique_ptr<std::thread> mTickThread;
	std::atomic<bool> mStopThread;
	std::mutex mStartThreadMutex;
	std::stringstream mConfStream;

	struct sFileName
	{
		std::string prefix;
		std::string suffix; // last number in path without ".gz"
	};

	boost::uintmax_t getFreeSpace();
	std::string getSuffix(const std::string &str);
	sFileName convertName(const std::string &fileName);
	std::chrono::system_clock::time_point lastWriteTime(const std::string &path);
	std::string getNextValueFromFile();
	bool parseConfFile();
	void reduce(); // rm last .gz or log file
	unsigned int getNumberOfLinesInFile(const std::string &filename);
	void tick();
	void rotate();
	bool needRotate();
	bool needReduce();
	void compressFile(const std::string &fname_in, const std::string &fname_out);
	std::chrono::system_clock::time_point getDateFromFilename(std::string filename);
	boost::uintmax_t getFilesSize(const std::vector<std::string> &fileVector);
};

#endif /* CLOGROTATE_H_ */
