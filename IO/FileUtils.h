#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>
#include <vector>
#include <unordered_map>

#include "BASE/Honey.h"
#include "BASE/HData.h"
#include "BASE/HValue.h"

namespace IO
{
    class FileUtils
    {
    public:
        static FileUtils &getInstance();
        virtual ~FileUtils() {}

        FILE *openFile(const std::string& filename, const std::string &mode);

        std::string getStringFromFile(const std::string& filename);
        HData getDataFromFile(const std::string& filename);

        void writeStringToFile(std::string dataStr, const std::string& fullPath);
        void writeDataToFile(HData retData, const std::string& fullPath);

        std::string getFilenameForNick(const std::string &filename) const;
        std::string fullPathForFilename(const std::string &filename) const;
        std::string getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const;
        std::string getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const;

        virtual bool isAbsolutePath(const std::string& path) const;
        bool isFileExist(const std::string& filename) const;
        bool isDirectoryExist(const std::string& dirPath) const;

        virtual bool createDirectory(const std::string& dirPath) = 0;
        virtual bool removeDirectory(const std::string& dirPath) = 0;
        virtual bool removeFile(const std::string &filepath) = 0;

        long getFileSize(const std::string &filepath);

        void setFilenameLookupDictionary(const ValueMap& filenameLookupDict);
        void setSearchResolutionsOrder(const std::vector<std::string>& searchResolutionsOrder);
        void addSearchResolutionsOrder(const std::string &order,const bool front=false);
        void setSearchPaths(const std::vector<std::string>& searchPaths);
        void addSearchPath(const std::string & path, const bool front=false);

    protected:
        FileUtils() {}

        virtual std::string getSuitableFOpen(const std::string& filenameUtf8) const;
        virtual bool isFileExistInternal(const std::string& filename) const = 0;
        virtual bool isDirectoryExistInternal(const std::string& dirPath) const = 0;

    private:
        HData getData(const std::string& filename, const std::string &mode = "rb");

    private:
        ValueMap filenameLookupDict_;
        std::vector<std::string> searchResolutionsOrderArray_;
        std::vector<std::string> searchPathArray_;
        mutable std::unordered_map<std::string, std::string> fullPathCache_;
    };
}

#endif // FILEUTILS_H
