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
    #if defined(__FreeBSD__) || defined(__APPLE__)
        #define stat64 stat
    #endif

    // Hack
    #ifdef __SYMBIAN32__
    static inline int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
        struct dirent *readdir_entry;

        readdir_entry = readdir(dirp);
        if (readdir_entry == NULLPTR)
        {
            *result = NULLPTR;
            return errno;
        }

        *entry = *readdir_entry;
        *result = entry;
        return 0;
    }
    #endif

    FILE *OpenFile(const std::string &filename, const char *mode) {
        FILE *file = NULLPTR;
    #if defined(_WIN32) && defined(UNICODE)
        file = _wfopen(UTF8ToWString(filename).c_str(), UTF8ToWString(mode).c_str());
    #else
        file =  fopen(filename.c_str(), mode);
    #endif
        if (file == NULLPTR)
            throw _HException_("OpenFile failed", HException::IO);
        return file;
    }

    void WriteStringToFile(bool text_file, const std::string &str, const char *filename) {
        FILE *file = OpenFile(filename, text_file ? "w" : "wb");
        Size len = str.size();
        if (len != fwrite(str.data(), 1, str.size(), file))
        {
            fclose(file);
            throw _HException_("fwrite failed", HException::IO);
        }
        fclose(file);
    }

    void WriteDataToFile(bool text_file, const void* data, const unsigned int size, const char *filename) {
        FILE *file = OpenFile(filename, text_file ? "w" : "wb");
        Size len = size;
        if (len != fwrite(data, 1, len, file)) {
            fclose(file);
            throw _HException_("fwrite failed", HException::IO);
        }
        fclose(file);
    }

    uint64 GetSize(FILE *file) {
        // This will only support 64-bit when large file support is available.
        // That won't be the case on some versions of Android, at least.
    #if defined(ANDROID) || (defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS < 64)
        int fd = fileno(file);

        off64_t pos = lseek64(fd, 0, SEEK_CUR);
        off64_t size = lseek64(fd, 0, SEEK_END);
        if (size != pos && lseek64(fd, pos, SEEK_SET) != pos) {
            throw _HException_("lseek64 failed", HException::IO);
        }
        return size;
    #else
        uint64_t pos = ftello(file);
        if (fseek(file, 0, SEEK_END) != 0) {
            throw _HException_("fseek failed", HException::IO);
        }
        uint64_t size = ftello(file);
        // Reset the seek position to where it was when we started.
        if (size != pos && fseeko(file, pos, SEEK_SET) != 0) {
            throw _HException_("fseeko failed", HException::IO);
        }
        return size;
    #endif
    }

    void ReadFileToString(bool text_file, const char *filename, std::string &str) {
        FILE *file = OpenFile(filename, text_file ? "r" : "rb");
        Size len = (Size)GetSize(file);
        char *buf = new char[len + 1];
        buf[fread(buf, 1, len, file)] = 0;
        str = std::string(buf, len);
        fclose(file);
        delete[] buf;
    }


    void ReadDataFromFile(bool text_file, unsigned char* &data, const unsigned int size, const char *filename) {
        FILE *file = OpenFile(filename, text_file ? "r" : "rb");
        Size len = (Size)GetSize(file);
        if(len > size) {
            fclose(file);
            throw _HException_Normal("truncating length");
        }
        data[fread(data, 1, size, file)] = 0;
        fclose(file);
    }

    // The return is non-const because - why not?
    uint8 *ReadLocalFile(const char *filename, Size *size) {
        FILE *file = OpenFile(filename, "rb");
        Size fSize = (Size)GetSize(file);
        uint8 *contents = new uint8[fSize+1];
        fSize = fread(contents, 1, fSize, file);
        fclose(file);
        contents[fSize] = 0;
        *size = fSize;
        return contents;
    }

    #define DIR_SEP "/"
    #define DIR_SEP_CHR '\\'

    #ifndef METRO

    // Remove any ending forward slashes from directory paths
    // Modifies argument.
    static void StripTailDirSlashes(std::string &fname) {
        if (fname.length() > 1) {
            Size i = fname.length() - 1;
            while (fname[i] == DIR_SEP_CHR)
                fname[i--] = '\0';
        }
        return;
    }

    // Returns true if filename is a directory
    bool IsDirectory(const std::string &filename) {
        FileInfo info;
        GetFileInfo(filename.c_str(), &info);
        return info.isDirectory;
    }

    void GetFileInfo(const char *path, FileInfo *fileInfo) {
        // TODO: Expand relative paths?
        fileInfo->fullName = path;

    #ifdef _WIN32
        WIN32_FILE_ATTRIBUTE_DATA attrs;
        if (!GetFileAttributesExW(UTF8ToWString(path).c_str(), GetFileExInfoStandard, &attrs)) {
            throw _HException_Normal("GetFileAttributes failed");
        }
        fileInfo->size = (uint64)attrs.nFileSizeLow | ((uint64)attrs.nFileSizeHigh << 32);
        fileInfo->isDirectory = (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        fileInfo->isWritable = (attrs.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0;
        fileInfo->exists = true;
    #else
        struct stat64 file_info;

        std::string copy(path);
        StripTailDirSlashes(copy);

        int result = stat64(copy.c_str(), &file_info);

        if (result < 0) {
            throw _HException_Normal("GetFileAttributes failed");
        }

        fileInfo->isDirectory = S_ISDIR(file_info.st_mode);
        fileInfo->isWritable = false;
        fileInfo->size = file_info.st_size;
        fileInfo->exists = true;
        // HACK: approximation
        if (file_info.st_mode & 0200)
            fileInfo->isWritable = true;
    #endif
    }

    std::string GetFileExtension(const std::string &fn) {
        int pos = (int)fn.rfind(".");
        if (pos < 0) return "";
        std::string ext = fn.substr(pos+1);
        for (Size i = 0; i < ext.size(); i++)
        {
            ext[i] = tolower(ext[i]);
        }
        return ext;
    }

    bool FileInfo::operator <(const FileInfo &other) const {
        if (isDirectory && !other.isDirectory)
            return true;
        else if (!isDirectory && other.isDirectory)
            return false;
        if (strcasecmp(name.c_str(), other.name.c_str()) < 0)
            return true;
        else
            return false;
    }

    void DeleteFile(const char *file)
    {
    #ifdef _WIN32
        if (!::DeleteFile(UTF8ToWString(file).c_str())) {
            throw _HException_(StringFromFormat("Error deleting %s: %i", file, GetLastError()), HException::IO);
        }
    #else
        int err = unlink(file);
        if (err) {
            throw _HException_(StringFromFormat("Error unlinking %s: %i", file, err), HException::IO);
        }
    #endif
    }

    void DeleteDirectory(const char *dir) {
    #ifdef _WIN32
        if (!RemoveDirectory(UTF8ToWString(dir).c_str())) {
            throw _HException_(StringFromFormat("Error deleting directory %s: %i", dir, GetLastError()), HException::IO);
        }
    #else
        rmdir(dir);
    #endif
    }

    #endif

    std::string GetDirectory(const std::string &path) {
        if (path == "/")
            return path;
        int n = (int)path.size() - 1;
        while (n >= 0 && path[n] != '\\' && path[n] != '/')
            n--;
        std::string cutpath = n > 0 ? path.substr(0, n) : "";
        for (Size i = 0; i < cutpath.size(); i++)
        {
            if (cutpath[i] == '\\') cutpath[i] = '/';
        }
    #ifndef _WIN32
        if (!cutpath.size()) {
            return "/";
        }
    #endif
        return cutpath;
    }

    std::string GetFileName(std::string path) {
        Size off = GetDirectory(path).size() + 1;
        if (off < path.size())
            return path.substr(off);
        else
            return path;
    }

    void MakeDirectory(const std::string &path) {
    #ifdef _WIN32
        mkdir(path.c_str());
    #else
        mkdir(path.c_str(), 0777);
    #endif
    }
}
