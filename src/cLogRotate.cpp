/*
 * cLogRotate.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: robert
 */

#include "cLogRotate.h"

cLogRotate::cLogRotate(const std::string &confFileName)
:
		mStopThread(false)
{
	mConfigFile.open(confFileName);
	if (mConfigFile.is_open())
	{
		std::cout << "start read conf file " << confFileName << std::endl;
		std::string wordFromFile;
		while (mConfigFile >> wordFromFile)
		{
			mConfStream << wordFromFile << " ";
		}
	}
	else
	{
		std::cout << "Open file error" << std::endl;
	}
	if (!parseConfFile())
	{
		throw std::runtime_error("Config file parser error");
	}

	mConfigFile.close();

	mFileRegex = boost::regex(std::string(R"(.*)") + mInstance + mLogFileBaseRegex);
	mGZFileRegex = boost::regex(std::string(R"(.*)") + mInstance + mGZFileBaseRegex);
	/*
	std::cout << "mFileRegex " << mFileRegex << std::endl;
	std::cout << "mGZFileRegex " << mGZFileRegex << std::endl;

	std::vector<std::string> vec = getFileVector(mFileRegex); // XXX
	for (auto a : vec)
	{
		std::cout << a << std::endl;
	}
	std::cout << " GZ files" << std::endl;
	std::vector<std::string> vec2 = getFileVector(mGZFileRegex); // XXX
	for (auto a : vec2)
	{
		std::cout << a << std::endl;
	}

	getDateFromFilename(vec[0]);
	getDateFromFilename(vec2[0]);
	*/
	mStartThreadMutex.lock();
	mTickThread.reset(new std::thread([this]()
			{
				mStartThreadMutex.lock();
				std::cout << "start thread" << std::endl; // XXX
				while (mStopThread)
				{
					tick();
					std::this_thread::sleep_for(mSleepTime);
				}
			}));
}

cLogRotate::~cLogRotate()
{
	mStopThread = false;
	mStartThreadMutex.unlock();
	mTickThread->join();
}

void cLogRotate::run()
{
	mStartThreadMutex.unlock();
}

void cLogRotate::tick()
{
	if (needRotate())
	{
		rotate();
	}
	if (needReduce())
	{
		reduce();
	}
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
	if (files_to_rotate.size() == mMaxLogFiles)
	{
		// TODO make real .gz file
		std::string gzFilename(files_to_rotate[0]);
		std::cout << "gzFilename " << gzFilename << std::endl;
		std::remove(gzFilename.begin(), gzFilename.end(), '\"');
		std::cout << "gzFilename " << gzFilename << std::endl;
		while (std::isdigit(gzFilename.back()))
		{
			gzFilename.pop_back();
		}
		gzFilename += std::to_string(mMaxLogFiles + 1);
		gzFilename += ".gz";
		compressFile(files_to_rotate[0], gzFilename);
		std::cout << "convert " << files_to_rotate[0] << " to gz: " << gzFilename << std::endl;
		fs::rename(files_to_rotate[0], gzFilename);
		files_to_rotate.erase(files_to_rotate.begin());
	}
	for (auto file : files_to_rotate)
	{
		sFileName sfn = convertName(file);
		std::string newName = sfn.prefix + std::to_string(std::stoi(sfn.suffix) + 1);
		//std::cout << "file " << file << " => " << newName << std::endl; // XXX
		fs::rename(file, newName);
	}
}

bool cLogRotate::needRotate() // TODO time of create file
{
	std::vector<std::string> normalFilesVector = getFileVector(mFileRegex);
	boost::uintmax_t sizeOfLogs = 0;
	std::sort(normalFilesVector.begin(), normalFilesVector.end(), [this](const std::string &a, const std::string &b)
			{
				return std::stoi(getSuffix(a)) > std::stoi(getSuffix(b));
			});
	for (auto file : normalFilesVector)
	{
		sizeOfLogs += fs::file_size(file);
	}

	// size of .log.1
	if (fs::file_size(normalFilesVector.back()) >= mMaxLogsSize)
	{
		return true;
	}

	// number of line in .log.1
	if (getNumberOfLinesInFile(normalFilesVector.back()) >= mSingleLines)
	{
		return true;
	}

	if (getDateFromFilename(normalFilesVector.back()) + mSingleTime < std::chrono::system_clock::now())
	{
		return true;
	}

	return false;
}

bool cLogRotate::needReduce()
{
	std::vector<std::string> filesVector = getFileVector(mFileRegex);
	std::vector<std::string> gzFilesVector = getFileVector(mGZFileRegex);
	for (auto file : gzFilesVector)
	{
		std::cout << file << std::endl;
	}
	std::cout << "free sapce " << getFreeSpace() << std::endl;
	if (getFreeSpace() <= mMinDiscFreeSpace)
	{
		return true;
	}

	if (gzFilesVector.size() >= mMaxGZFiles)
	{
		return true;
	}
	return false;
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
	//mConfigFile >> value;
	//mConfigFile >> value;
	mConfStream >> value;
	mConfStream >> value;
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
		mSingleLines = std::stoi(getNextValueFromFile());
		mSingleTime = std::chrono::seconds(std::stoi(getNextValueFromFile()));
		mPath = getNextValueFromFile();
		mInstance = getNextValueFromFile();
		mMaxLogStorageTime = std::chrono::seconds(std::stoi(getNextValueFromFile()));
		mSleepTime = std::chrono::seconds(std::stoi(getNextValueFromFile()));

		return true;
	}
	catch (...)
	{
		return false;
	}
}

void cLogRotate::reduce() // TODO time of creation last file
{
	std::vector<std::string> gzFiles = getFileVector(mGZFileRegex);
	if (gzFiles.empty())
	{
		std::vector<std::string> logFiles = getFileVector(mFileRegex);
		if (logFiles.empty())
		{
			return;
		}
		std::sort(logFiles.begin(), logFiles.end(), [this](const std::string &a, const std::string &b)
				{
					return std::stoi(getSuffix(a)) > std::stoi(getSuffix(b));
				});
		fs::remove(logFiles.front());
		return;
	}
	// rm last .gz file
	std::sort(gzFiles.begin(), gzFiles.end(), [this](const std::string &a, const std::string &b)
			{
				std::string aSuffix(a);
				std::string bSuffix(b);
				aSuffix.erase(aSuffix.end() - sizeOfFileType, aSuffix.end());
				bSuffix.erase(bSuffix.end() - sizeOfFileType, bSuffix.end());
				return std::stoi(getSuffix(aSuffix)) > std::stoi(getSuffix(bSuffix));
			});
	fs::remove(gzFiles.front());
}

unsigned int cLogRotate::getNumberOfLinesInFile(const std::string &filename)
{
	std::string line;
	std::ifstream inputFile(filename);
	unsigned int numberOfLines = 0;
	while (std::getline(inputFile, line))
	{
		numberOfLines++;
	}
	inputFile.close();
	return numberOfLines;
}

void cLogRotate::compressFile(const std::string &fname_in, const std::string &fname_out)
{
	const size_t compress_buffer_size = 1024*128;
	const size_t copy_buffer_size = 1024*128;
	gzFile gz_file = gzopen( fname_out.c_str(), "wb9");
	if ( gz_file == NULL) throw std::runtime_error( std::string("gzip: can not open file '") + fname_out + std::string("'") );
	if ( gzbuffer(gz_file , compress_buffer_size)  != 0) throw std::runtime_error("gzip: can not set gzip compress size");

	std::ifstream input_file;
	input_file.open(fname_in.c_str(), std::ios::binary);

	char buf[copy_buffer_size];
	input_file.read(buf, copy_buffer_size);
	unsigned int len_read = input_file.gcount();
	if (len_read) {
		unsigned int len_written = gzwrite(gz_file, buf, len_read);
		if (len_written != len_read) throw std::runtime_error("gzip: can not write some of the data");
	}
	gzclose(gz_file);
}

std::chrono::system_clock::time_point cLogRotate::getDateFromFilename(std::string filename)
{
	if (filename.back() == 'z') // *.gz
	{
		filename.erase(filename.end() - sizeOfFileType, filename.end());
	}

	while (filename.back() != '.')
	{
		filename.pop_back();
	}
	filename.pop_back();
	while (filename.back() != '.')
	{
		filename.pop_back();
	}
	filename.pop_back();
	//std::stringstream ss(filename.substr(filename.size() - 19));
	/*std::tm tm;
	ss >> std::get_time(&tm, "%Y.%m.%d-%H.%M.%S");*/
	//2015.04.27-17.05.00
	const std::locale format(std::locale::classic(), new boost::posix_time::time_input_facet("%Y.%m.%d-%H.%M.%S"));
	std::stringstream ss(filename.substr(filename.size() - 19));
	ss.imbue(format);
	boost::posix_time::ptime ptime;
	ss >> ptime;
	std::tm tm = boost::posix_time::to_tm(ptime);
	return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

//																	rrrr    MM     dd    hh     mm     ss
const std::string cLogRotate::mLogFileBaseRegex = 	std::string(R"(\.\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}\.log\.\d+)"); // /path/.../xyz.2000.01.31-12.00.00.log.1234
const std::string cLogRotate::mGZFileBaseRegex = 	std::string(R"(\.\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}\.log\.\d+\.gz)"); // /path/.../xyz.2000.01.31-12.00.00.log.5678.gz
