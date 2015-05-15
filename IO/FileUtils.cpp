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

#include "UTILS/TEXT/UTF8.h"
using UTILS::TEXT::ConvertUTF8ToWString;
#ifdef _WIN32
using UTILS::TEXT::ConvertWStringToUTF8;
#endif
#include "UTILS/STRING/String.h"
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
        file = _wfopen(ConvertUTF8ToWString(filename).c_str(), ConvertUTF8ToWString(mode).c_str());
    #else
        file =  fopen(filename.c_str(), mode);
    #endif
        if (file == NULLPTR)
            throw _NException_("OpenFile failed", NException::IO);
        return file;
    }

    void WriteStringToFile(bool text_file, const std::string &str, const char *filename) {
        FILE *file = OpenFile(filename, text_file ? "w" : "wb");
        Size len = str.size();
        if (len != fwrite(str.data(), 1, str.size(), file))
        {
            fclose(file);
            throw _NException_("fwrite failed", NException::IO);
        }
        fclose(file);
    }

    void WriteDataToFile(bool text_file, const void* data, const unsigned int size, const char *filename) {
        FILE *file = OpenFile(filename, text_file ? "w" : "wb");
        Size len = size;
        if (len != fwrite(data, 1, len, file)) {
            fclose(file);
            throw _NException_("fwrite failed", NException::IO);
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
            throw _NException_("lseek64 failed", NException::IO);
        }
        return size;
    #else
        uint64_t pos = ftello(file);
        if (fseek(file, 0, SEEK_END) != 0) {
            throw _NException_("fseek failed", NException::IO);
        }
        uint64_t size = ftello(file);
        // Reset the seek position to where it was when we started.
        if (size != pos && fseeko(file, pos, SEEK_SET) != 0) {
            throw _NException_("fseeko failed", NException::IO);
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
            throw _NException_Normal("truncating length");
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
        if (!GetFileAttributesExW(ConvertUTF8ToWString(path).c_str(), GetFileExInfoStandard, &attrs)) {
            throw _NException_Normal("GetFileAttributes failed");
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
            throw _NException_Normal("GetFileAttributes failed");
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

    Size GetFilesInDirectory(const char *directory, std::vector<FileInfo> *files, const char *filter, int flags) {
        Size foundEntries = 0;
        std::set<std::string> filters;
        std::string tmp;
        if (filter) {
            while (*filter) {
                if (*filter == ':') {
                    filters.insert(tmp);
                    tmp = "";
                }
                else {
                    tmp.push_back(*filter);
                }
                filter++;
            }
        }
        if (tmp.size())
            filters.insert(tmp);
    #ifdef _WIN32
        // Find the first file in the directory.
        WIN32_FIND_DATA ffd;
    #ifdef UNICODE

        HANDLE hFind = FindFirstFile((ConvertUTF8ToWString(directory) + L"\\*").c_str(), &ffd);
    #else
        HANDLE hFind = FindFirstFile((std::string(directory) + "\\*").c_str(), &ffd);
    #endif
        if (hFind == INVALID_HANDLE_VALUE) {
            FindClose(hFind);
            return 0;
        }
        // windows loop
        do {
            const std::string virtualName = ConvertWStringToUTF8(ffd.cFileName);
    #else
        struct dirent_large { struct dirent entry; char padding[FILENAME_MAX+1]; };
        struct dirent_large diren;
        struct dirent *result = NULLPTR;

        //std::string directoryWithSlash = directory;
        //if (directoryWithSlash.back() != '/')
        //	directoryWithSlash += "/";

        DIR *dirp = opendir(directory);
        if (!dirp)
            return 0;
        // non windows loop
        while (!readdir_r(dirp, (dirent*) &diren, &result) && result) {
            const std::string virtualName(result->d_name);
    #endif
            // check for "." and ".."
            if (((virtualName[0] == '.') && (virtualName[1] == '\0')) ||
                ((virtualName[0] == '.') && (virtualName[1] == '.') &&
                (virtualName[2] == '\0')))
                continue;

            // Remove dotfiles (should be made optional?)
            if (!(flags & GETFILES_GETHIDDEN) && virtualName[0] == '.')
                continue;

            FileInfo info;
            info.name = virtualName;
            std::string dir = directory;

            // Only append a slash if there isn't one on the end.
            Size lastSlash = dir.find_last_of("/");
            if (lastSlash != (dir.length() - 1))
                dir.append("/");

            info.fullName = dir + virtualName;
            info.isDirectory = IsDirectory(info.fullName);
            info.exists = true;
            info.size = 0;
            if (!info.isDirectory) {
                std::string ext = GetFileExtension(info.fullName);
                if (filter) {
                    if (filters.find(ext) == filters.end())
                        continue;
                }
            }

            if (files)
                files->push_back(info);
            foundEntries++;
    #ifdef _WIN32
        } while (FindNextFile(hFind, &ffd) != 0);
        FindClose(hFind);
    #else
        }
        closedir(dirp);
    #endif
        if (files)
            std::sort(files->begin(), files->end());
        return foundEntries;
    }

    void DeleteFile(const char *file)
    {
    #ifdef _WIN32
        if (!::DeleteFile(ConvertUTF8ToWString(file).c_str())) {
            throw _NException_(StringFromFormat("Error deleting %s: %i", file, GetLastError()), NException::IO);
        }
    #else
        int err = unlink(file);
        if (err) {
            throw _NException_(StringFromFormat("Error unlinking %s: %i", file, err), NException::IO);
        }
    #endif
    }

    void DeleteDirectory(const char *dir) {
    #ifdef _WIN32
        if (!RemoveDirectory(ConvertUTF8ToWString(dir).c_str())) {
            throw _NException_(StringFromFormat("Error deleting directory %s: %i", dir, GetLastError()), NException::IO);
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

    // Returns true if file filename exists
    bool Exists(const std::string &filename) {
    #ifdef _WIN32
        std::wstring wstr = ConvertUTF8ToWString(filename);
        return GetFileAttributes(wstr.c_str()) != 0xFFFFFFFF;
    #else
        struct stat64 file_info;

        std::string copy(filename);
        StripTailDirSlashes(copy);

        int result = stat64(copy.c_str(), &file_info);

        return (result == 0);
    #endif
    }

    #ifdef _WIN32
    // Returns a vector with the device names
    std::vector<std::string> GetWindowsDrives() {
        std::vector<std::string> drives;

        const DWORD buffsize = GetLogicalDriveStrings(0, NULLPTR);
        std::vector<TCHAR> buff(buffsize);
        if (GetLogicalDriveStrings(buffsize, buff.data()) == buffsize - 1) {
            auto drive = buff.data();
            while (*drive) {
                std::string str(ConvertWStringToUTF8(drive));
                str.pop_back();	// we don't want the final backslash
                str += "/";
                drives.push_back(str);

                // advance to next drive
                while (*drive++) {}
            }
        }
        return drives;
    }
    #endif
}
