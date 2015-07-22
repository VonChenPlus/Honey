#include "FileUtils.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <direct.h>
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#define fseeko _fseeki64
#define ftello _ftelli64
#else
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#endif
#include <cstring>
#include <string>
#include <set>
#include <algorithm>
#include <cstdio>
#include <sys/stat.h>
#include <ctype.h>

#include "UTILS/STRING/UTFUtils.h"
using UTILS::STRING::UTF8ToWString;
#include "UTILS/STRING/StringUtils.h"
using UTILS::STRING::StringFromFormat;

namespace IO
{
    #ifdef _WIN32
    class FileUtilsWin final : public FileUtils
    {
    public:
        bool createDirectory(const std::string& dirPath) override {
            return mkdir(dirPath.c_str()) == 0;
        }

        bool removeDirectory(const std::string& dirPath) {
            return !!::RemoveDirectory(UTF8ToWString(dirPath).c_str());
        }

        bool removeFile(const std::string &filepath) {
            return !!::DeleteFile(UTF8ToWString(filepath).c_str());
        }

    private:
        std::string getSuitableFOpen(const std::string& filenameUtf8) const override {
            std::string string;
            UTILS::STRING::UTF8ToString(filenameUtf8, string);
            return string;
        }

        bool isFileExistInternal(const std::string& strFilePath) const override {
            if (strFilePath.empty()) {
                return false;
            }

            DWORD attr = GetFileAttributesW(UTF8ToWString(strFilePath).c_str());
            if(attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
                return false;   //  not a file
            return true;
        }

        bool isDirectoryExistInternal(const std::string& dirPath) const override {
            unsigned long fAttrib = GetFileAttributesW(UTF8ToWString(dirPath).c_str());
            if (fAttrib != INVALID_FILE_ATTRIBUTES &&
                (fAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
                return true;
            }
            return false;
        }
    };
    #else
    class FileUtilsLinux final : public FileUtils
    {
    public:
        bool createDirectory(const std::string& dirPath) override {
            mkdir(path.c_str(), 0777);
        }

        bool removeDirectory(const std::string& dirPath) {
            rmdir(dir);
        }

        bool removeFile(const std::string &filepath) {
            return unlink(file) == 0;
        }

    private:
        bool isFileExistInternal(const std::string &strFilePath) const override {
            if (strFilePath.empty()) {
                return false;
            }

            struct stat sts;
            return (stat(strFilePath.c_str(), &sts) != -1) ? true : false;
        }

        bool isDirectoryExistInternal(const std::string& dirPath) const override{
            if (strFilePath.empty()) {
                return false;
            }

            struct stat sts;
            int result = stat(strFilePath.c_str(), &sts);
            if (result < 0)
                return false;

            return S_ISDIR(sts.st_mode);
        }
    };
    #endif

    FileUtils &FileUtils::getInstance() {
        #ifdef _WIN32
        static FileUtilsWin instance;
        #else
        static FileUtilsLinux instance;
        #endif
        return instance;
    }

    FILE *FileUtils::openFile(const std::string& filename, const std::string &mode) {
        if (filename.empty() || mode.empty()) {
            throw _HException_Normal("FileUitls::getData Params Error!");
        }

        do {
            // Read the file from hardware
            std::string fullPath = fullPathForFilename(filename);
            FILE *fp = fopen(getSuitableFOpen(fullPath).c_str(), mode.c_str());
            if (fp == NULLPTR) {
                throw _HException_("fopen failed", HException::IO);
            }
            return fp;
        } while(0);

        return NULLPTR;
    }

    std::string FileUtils::getStringFromFile(const std::string& filename) {
        HData data = getData(filename, "rt");
        if (data.isNull())
            return "";

        std::string ret((const char*)data.getBytes());
        ret.append('\0');
        return ret;
    }

    void FileUtils::writeStringToFile(std::string dataStr, const std::string& fullPath) {
        HData retData;
        retData.copy((HBYTE *)dataStr.c_str(), dataStr.size());

        return writeDataToFile(retData, fullPath);
    }


    void FileUtils::writeDataToFile(HData retData, const std::string& fullPath) {
        if (retData.isNull() || fullPath.empty()) {
            throw _HException_Normal("FileUitls::writeDataToFile Params Error!");
        }

        size_t size = 0;
        const char* mode = "wb";

        do {
            // Read the file from hardware
            FILE *fp = fopen(getSuitableFOpen(fullPath).c_str(), mode);
            if (fp == NULLPTR) {
                throw _HException_("fopen failed", HException::IO);
            }

            size = retData.getSize();
            fwrite(retData.getBytes(), size, 1, fp);
            fclose(fp);
        } while (0);
    }

    HData FileUtils::getDataFromFile(const std::string& filename) {
        return getData(filename);
    }

    std::string FileUtils::getFilenameForNick(const std::string &nickname) const {
        std::string newFileName;

        // in Lookup Filename dictionary ?
        auto iter = filenameLookupDict_.find(nickname);

        if (iter == filenameLookupDict_.end()) {
            newFileName = nickname;
        }
        else {
            newFileName = iter->second.asString();
        }

        return newFileName;
    }

    std::string FileUtils::fullPathForFilename(const std::string &filename) const {
        if (filename.empty()){
            throw _HException_Normal("filename is Empty!");
        }

        if (isAbsolutePath(filename)) {
            return filename;
        }

        // Already Cached ?
        auto cacheIter = fullPathCache_.find(filename);
        if(cacheIter != fullPathCache_.end()) {
            return cacheIter->second;
        }

        // Get the new file name.
        const std::string newFilename(getFilenameForNick(filename));
        std::string fullpath;

        for (const auto& searchIt : searchPathArray_) {
            for (const auto& resolutionIt : searchResolutionsOrderArray_) {
                fullpath = getPathForFilename(newFilename, resolutionIt, searchIt);
                if (fullpath.length() > 0) {
                    // Using the filename passed in as key.
                    fullPathCache_.insert(std::make_pair(filename, fullpath));
                    return fullpath;
                }
            }
        }

        return "";
    }

    std::string FileUtils::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const {
        std::string file = filename;
        std::string file_path = "";
        size_t pos = filename.find_last_of("/");
        if (pos != std::string::npos) {
            file_path = filename.substr(0, pos+1);
            file = filename.substr(pos+1);
        }

        // searchPath + file_path + resourceDirectory
        std::string path = searchPath;
        path += file_path;
        path += resolutionDirectory;

        path = getFullPathForDirectoryAndFilename(path, file);

        return path;
    }

    std::string FileUtils::getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const {
        // get directory+filename, safely adding '/' as necessary
        std::string ret = directory;
        if (directory.size() && directory[directory.size()-1] != '/') {
            ret += '/';
        }
        ret += filename;

        // if the file doesn't exist, return an empty string
        if (!isFileExistInternal(ret)) {
            ret = "";
        }
        return ret;
    }

    bool FileUtils::isAbsolutePath(const std::string& path) const {
        return (path[0] == '/');
    }

    bool FileUtils::isFileExist(const std::string& filename) const {
        if (isAbsolutePath(filename)) {
            return isFileExistInternal(filename);
        }
        else {
            std::string fullpath = fullPathForFilename(filename);
            if (fullpath.empty())
                return false;
            else
                return true;
        }
    }

    bool FileUtils::isDirectoryExist(const std::string& dirPath) const {
        if (isAbsolutePath(dirPath)) {
            return isDirectoryExistInternal(dirPath);
        }

        // Already Cached ?
        auto cacheIter = fullPathCache_.find(dirPath);
        if( cacheIter != fullPathCache_.end() ) {
            return isDirectoryExistInternal(cacheIter->second);
        }

        std::string fullpath;
        for (const auto& searchIt : searchPathArray_) {
            for (const auto& resolutionIt : searchResolutionsOrderArray_) {
                // searchPath + file_path + resourceDirectory
                fullpath = searchIt + dirPath + resolutionIt;
                if (isDirectoryExistInternal(fullpath))
                {
                    fullPathCache_.insert(std::make_pair(dirPath, fullpath));
                    return true;
                }
            }
        }
        return false;
    }

    long FileUtils::getFileSize(const std::string &filepath) {
        std::string fullpath = filepath;
        if (!isAbsolutePath(filepath))
        {
            fullpath = fullPathForFilename(filepath);
            if (fullpath.empty())
                return 0;
        }

        struct stat info;
        // Get data associated with "crt_stat.c":
        int result = stat( fullpath.c_str(), &info );

        // Check if statistics are valid:
        if( result != 0 ) {
            // Failed
            return -1;
        }
        else {
            return (long)(info.st_size);
        }
    }

    void FileUtils::setFilenameLookupDictionary(const ValueMap& filenameLookupDict) {
        fullPathCache_.clear();
        filenameLookupDict_ = filenameLookupDict;
    }

    void FileUtils::setSearchResolutionsOrder(const std::vector<std::string>& searchResolutionsOrder) {
        bool existDefault = false;
        fullPathCache_.clear();
        searchResolutionsOrderArray_.clear();
        for(const auto& iter : searchResolutionsOrder)
        {
            std::string resolutionDirectory = iter;
            if (!existDefault && resolutionDirectory == "") {
                existDefault = true;
            }

            if (resolutionDirectory.length() > 0 && resolutionDirectory[resolutionDirectory.length()-1] != '/') {
                resolutionDirectory += "/";
            }

            searchResolutionsOrderArray_.push_back(resolutionDirectory);
        }

        if (!existDefault) {
            searchResolutionsOrderArray_.push_back("");
        }
    }

    void FileUtils::addSearchResolutionsOrder(const std::string &order,const bool front) {
        std::string resOrder = order;
        if (!resOrder.empty() && resOrder[resOrder.length()-1] != '/')
            resOrder.append("/");

        if (front) {
            searchResolutionsOrderArray_.insert(searchResolutionsOrderArray_.begin(), resOrder);
        }
        else {
            searchResolutionsOrderArray_.push_back(resOrder);
        }
    }

    void FileUtils::setSearchPaths(const std::vector<std::string>& searchPaths) {
        fullPathCache_.clear();
        searchPathArray_.clear();
        for (const auto& iter : searchPaths) {
            std::string prefix;
            std::string path;

            path = prefix + (iter);
            if (path.length() > 0 && path[path.length()-1] != '/') {
                path += "/";
            }
            searchPathArray_.push_back(path);
        }
    }

    void FileUtils::addSearchPath(const std::string &searchpath,const bool front) {
        std::string prefix;

        std::string path = prefix + searchpath;
        if (path.length() > 0 && path[path.length()-1] != '/') {
            path += "/";
        }
        if (front) {
            searchPathArray_.insert(searchPathArray_.begin(), path);
        }
        else {
            searchPathArray_.push_back(path);
        }
    }

    std::string FileUtils::getSuitableFOpen(const std::string& filenameUtf8) const {
        return filenameUtf8;
    }

    HData FileUtils::getData(const std::string& filename, const std::string &mode) {
        if (filename.empty() || mode.empty()) {
            throw _HException_Normal("FileUitls::getData Params Error!");
        }

        HData ret;
        HBYTE* buffer = NULLPTR;
        Size size = 0;
        Size readSize = 0;

        do {
            // Read the file from hardware
            std::string fullPath = fullPathForFilename(filename);
            FILE *fp = fopen(getSuitableFOpen(fullPath).c_str(), mode.c_str());
            if (fp == NULLPTR) {
                throw _HException_("fopen failed", HException::IO);
            }

            fseek(fp,0,SEEK_END);
            size = ftell(fp);
            fseek(fp,0,SEEK_SET);

            buffer = (HBYTE*)malloc(sizeof(HBYTE) * size);

            readSize = fread(buffer, sizeof(unsigned char), size, fp);
            fclose(fp);
        } while (0);

        if (NULLPTR == buffer || 0 == readSize) {
            throw _HException_("Get data from file failed", HException::IO);
        }
        else {
            ret.fastSet(buffer, readSize);
        }

        return ret;
    }
}
