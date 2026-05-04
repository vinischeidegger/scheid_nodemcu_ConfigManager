#ifndef MOCK_LITTLEFS_H
#define MOCK_LITTLEFS_H

#include "mock_arduino.h"

namespace LittleFS {
    class File {
    public:
        File() = default;
        operator bool() const { return true; }
        void close() {}
    };

    inline bool begin() { return true; }
    inline void format() {}
    inline File open(const char* path, const char* mode) { return File(); }
}

#endif // MOCK_LITTLEFS_H
