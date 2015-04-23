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
  mMaxLogStorageTime (maxLogStorageTime),
  mFileRegex(std::string(R"(.*\.log\.\d+)")),
  mGZFileRegex(std::string(R"(.*\.log\.\d+\.gz)"))
{
}

cLogRotate::~cLogRotate()
{
}

void cLogRotate::rotate()
{
	std::vector<std::string> files_to_rotate = getFileVector(mFileRegex);
	std::sort(files_to_rotate.begin(), files_to_rotate.end(), [this](const std::string &a, const std::string &b)
			{
				return std::stoi(getSuffix(a)) < std::stoi(getSuffix(b));
			});

	for (auto file : files_to_rotate)
	{
		sFileName sfn = convertName(file);
		std::cout << "file " << file << " => " << sfn.prefix << std::stoi(sfn.suffix) + 1 << std::endl; // XXX
	}

	// gz files
	std::vector<std::string> gz_files_to_rotate = getFileVector(mGZFileRegex);
	std::sort(gz_files_to_rotate.begin(), gz_files_to_rotate.end(), [this](const std::string &a, const std::string &b)
			{
				std::string aSuffix(a);
				std::string bSuffix(b);
				aSuffix.erase(aSuffix.end() - 3, aSuffix.end());
				bSuffix.erase(bSuffix.end() - 3, bSuffix.end());
				return std::stoi(getSuffix(aSuffix)) < std::stoi(getSuffix(bSuffix));
			});
	for (auto file : gz_files_to_rotate) // XXX
	{
		sFileName sfn = convertName(file);
		//std::cout << "gz file prefix " << sfn.prefix << " suffix " << sfn.suffix << std::endl; // XXX
		std::cout << "gz file " << file << " => " << sfn.prefix << std::stoi(sfn.suffix) + 1 << ".gz" << std::endl; // XXX
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
	const unsigned int sizeOfFileType = std::string(".gz").size();
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
