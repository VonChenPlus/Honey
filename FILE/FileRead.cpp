#include "FileRead.h"

namespace FILE
{
    // The return is non-const because - why not?
    uint8_t *ReadLocalFile(const char *filename, size_t *size)
    {
        FILE *file = fopen(filename, "rb");
        if (!file) {
            return 0;
        }
        fseek(file, 0, SEEK_END);
        size_t f_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        uint8_t *contents = new uint8_t[f_size+1];
        fread(contents, 1, f_size, file);
        fclose(file);
        contents[f_size] = 0;
        *size = f_size;
        return contents;
    }
}

