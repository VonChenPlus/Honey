#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string>
#include <vector>

#include "BASE/BasicTypes.h"

namespace IO
{
    // Whole-file reading/writing
    bool WriteStringToFile(bool text_file, const std::string &str, const char *filename);
    bool ReadFileToString(bool text_file, const char *filename, std::string &str);

    bool WriteDataToFile(bool text_file, const void* data, const unsigned int size, const char *filename);
    bool ReadDataFromFile(bool text_file, unsigned char* &data, const unsigned int size, const char *filename);

    // Direct readers. deallocate using delete [].
    uint8 *ReadLocalFile(const char *filename, Size *size);
    // Beginnings of a directory utility system. TODO: Improve.

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
    std::string GetDir(const std::string &path);
    std::string GetFilename(std::string path);
    bool GetFileInfo(const char *path, FileInfo *fileInfo);

    enum
    {
        GETFILES_GETHIDDEN = 1
    };

    Size GetFilesInDir(const char *directory, std::vector<FileInfo> *files, const char *filter = 0, int flags = 0);
    void DeleteFile(const char *file);
    void DeleteDir(const char *file);
    bool Exists(const std::string &filename);
    bool IsDirectory(const std::string &filename);
    void MakeDir(const std::string &path);
    std::string GetDir(const std::string &path);

    #ifdef _WIN32
    std::vector<std::string> GetWindowsDrives();
    #endif
}

#endif // FILEUTIL_H
