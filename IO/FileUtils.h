#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>
#include <vector>
#include <unordered_map>

#include "BASE/Honey.h"
#include "BASE/HData.h"

namespace IO
{
    // File IO
    FILE *OpenFile(const std::string &filename, const char *mode);

    // Whole-file reading/writing
    void WriteStringToFile(bool text_file, const std::string &str, const char *filename);
    void ReadFileToString(bool text_file, const char *filename, std::string &str);

    void WriteDataToFile(bool text_file, const void* data, const unsigned int size, const char *filename);
    void ReadDataFromFile(bool text_file, unsigned char* &data, const unsigned int size, const char *filename);

    // Direct readers. deallocate using delete [].
    uint8 *ReadLocalFile(const char *filename, Size *size);

    // File Operator
    struct FileInfo
    {
        std::string name;
        std::string fullName;
        bool exists;
        bool isDirectory;
        bool isWritable;
        uint64 size;
        bool operator <(const FileInfo &other) const;
    };

    std::string GetFileExtension(const std::string &fn);
    std::string GetFileName(std::string path);
    void GetFileInfo(const char *path, FileInfo *fileInfo);

    // Directory Operator
    enum
    {
        GETFILES_GETHIDDEN = 1
    };

    void DeleteFile(const char *file);
    void DeleteDirectory(const char *file);
    bool IsDirectory(const std::string &filename);
    void MakeDirectory(const std::string &path);
    std::string GetDirectory(const std::string &path);

    class FileUtils
    {
    public:
        virtual ~FileUtils();

        virtual std::string getStringFromFile(const std::string& filename);

        virtual std::string fullPathForFilename(const std::string &filename) const;

        virtual bool isAbsolutePath(const std::string& path) const;

    private:
        HData getData(const std::string& filename, const std::string &mode);

    private:
        mutable std::unordered_map<std::string, std::string> fullPathCache_;
    };
}

#endif // FILEUTILS_H
