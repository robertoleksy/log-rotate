/*
 * cLogRotate.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: robert
 */

#include "cLogRotate.h"

cLogRotate::cLogRotate()
:
  mMaxLogFiles(5),
  mMaxGZFiles(10),
  mMinDiscFreeSpace(10 * 1024 * 1024), // 10MB
  mMaxLogsSize(10 * 1024 * 1024),
  mPath ("."),
  mMaxLogStorageTime(std::chrono::hours(24 * 30)),
  mSleepTime(10 * 60),
  mInstance("test")
{
	mFileRegex = boost::regex(std::string(R"(.*)") + mInstance + mLogFileBaseRegex);
	mGZFileRegex = boost::regex(std::string(R"(.*)") + mInstance + mGZFileBaseRegex);
}

cLogRotate::cLogRotate(const std::string &confFileName)
{
	mConfigFile.open("test.conf");
	if (!mConfigFile.is_open())
	{
		throw std::runtime_error("Config file onen error");
	}
	else if (!parseConfFile())
	{
		throw std::runtime_error("Config file parser error");
	}

	mFileRegex = boost::regex(std::string(R"(.*)") + mInstance + mLogFileBaseRegex);
	mGZFileRegex = boost::regex(std::string(R"(.*)") + mInstance + mGZFileBaseRegex);

	std::cout << "mFileRegex " << mFileRegex << std::endl;
	std::cout << "mGZFileRegex " << mGZFileRegex << std::endl;
}

cLogRotate::~cLogRotate()
{
}

void cLogRotate::rotate()
{
	// gz files
	std::vector<std::string> gz_files_to_rotate = getFileVector(mGZFileRegex);
	std::sort(gz_files_to_rotate.begin(), gz_files_to_rotate.end(), [this](const std::string &a, const std::string &b)
			{
				std::string aSuffix(a);
				std::string bSuffix(b);
				aSuffix.erase(aSuffix.end() - sizeOfFileType, aSuffix.end());
				bSuffix.erase(bSuffix.end() - sizeOfFileType, bSuffix.end());
				return std::stoi(getSuffix(aSuffix)) > std::stoi(getSuffix(bSuffix));
			});
	for (auto file : gz_files_to_rotate)
	{
		sFileName sfn = convertName(file);
		//std::cout << "gz file prefix " << sfn.prefix << " suffix " << sfn.suffix << std::endl; // XXX
		std::string newName = sfn.prefix + std::to_string(std::stoi(sfn.suffix) + 1) + ".gz";
		//std::cout << "gz file " << file << " => " << newName << std::endl; // XXX
		//std::time_t tt = std::chrono::system_clock::to_time_t(lastWriteTime(file)); // XXX
		//std::cout << "file " << file << " " << std::ctime(&tt) << std::endl; // XXX
		fs::rename(file, newName);
	}

	// normal logs
	std::vector<std::string> files_to_rotate = getFileVector(mFileRegex);
	std::sort(files_to_rotate.begin(), files_to_rotate.end(), [this](const std::string &a, const std::string &b)
			{
				return std::stoi(getSuffix(a)) > std::stoi(getSuffix(b));
			});

	for (auto file : files_to_rotate)
	{
		sFileName sfn = convertName(file);
		std::string newName = sfn.prefix + std::to_string(std::stoi(sfn.suffix) + 1);
		std::cout << "file " << file << " => " << newName << std::endl; // XXX
		fs::rename(file, newName);
	}
}

boost::uintmax_t cLogRotate::getFreeSpace()
{
	fs::space_info space_inf = fs::space(mPath);
	return space_inf.available;
}

std::vector<std::string> cLogRotate::getFileVector(const boost::regex &fileRegex)
{
	fs::directory_iterator end_iter;
	std::string fileName;
	std::vector<std::string> fileVector;

	for( fs::directory_iterator dir_iter(mPath) ; dir_iter != end_iter ; ++dir_iter)
	{
		if (fs::is_regular_file(dir_iter->status()))
		{
			fileName = dir_iter->path().c_str();
			if (boost::regex_match(fileName, fileRegex))
			{
				fileVector.emplace_back(std::move(fileName));
			}
		}
	}

	return fileVector;
}

std::string cLogRotate::getSuffix(const std::string &str)
{
	std::string suffix;
	auto it = str.end() - 1;
	while (*it != '.')
	{
		suffix.insert(suffix.begin(), *it);
		it--;
	}

	return suffix;
}

cLogRotate::sFileName cLogRotate::convertName(const std::string &fileName)
{
	sFileName sName;
	if (fileName.substr(fileName.size() - sizeOfFileType) == ".gz")
	{
		sName.suffix = getSuffix(fileName.substr(0, fileName.size() - sizeOfFileType));
		sName.prefix = fileName.substr(0, fileName.size() - sName.suffix.size() - sizeOfFileType);
	}
	else
	{
		sName.suffix = getSuffix(fileName);
		sName.prefix = fileName.substr(0, fileName.size() - sName.suffix.size());
	}
	return sName;
}

std::chrono::system_clock::time_point cLogRotate::lastWriteTime(const std::string &path)
{
	std::time_t writeTime_t = fs::last_write_time(path);
	return std::chrono::system_clock::from_time_t(writeTime_t);
}

std::string cLogRotate::getNextValueFromFile()
{
	std::string value;
	mConfigFile >> value;
	mConfigFile >> value;
	return value;
}

bool cLogRotate::parseConfFile()
{
	try
	{
		mMaxLogFiles = std::stoi(getNextValueFromFile());
		mMaxGZFiles = std::stoi(getNextValueFromFile());
		mMinDiscFreeSpace = std::stoi(getNextValueFromFile());
		mMaxLogsSize = std::stoi(getNextValueFromFile());
		mPath = getNextValueFromFile();
		mInstance = getNextValueFromFile();
		mMaxLogStorageTime = std::chrono::hours(std::stoi(getNextValueFromFile()));
		mSleepTime = std::chrono::seconds(std::stoi(getNextValueFromFile()));

		return true;
	}
	catch (...)
	{
		return false;
	}
}

const std::string cLogRotate::mLogFileBaseRegex = std::string(R"(\.log\.\d+)"); // /path/.../xxx.log.1234
const std::string cLogRotate::mGZFileBaseRegex = std::string(R"(\.log\.\d+\.gz)"); // /path/.../xxx.log.5678.gz
