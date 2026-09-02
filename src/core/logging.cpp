#include "logging.hpp"

#include <stdio.h>
#include <string.h>

namespace LogImpl {
    static int indentLevel = 0;

    void Indent() {
        ++indentLevel;
    }

    void Unindent() {
        if (indentLevel > 0)
            --indentLevel;
    }

    int GetIndent() {
        return indentLevel;
    }

    const char *GetFilename(const char *path) {
        const char* slash = strrchr(path, '/');
        const char* backslash = strrchr(path, '\\');
        const char* separator = slash > backslash ? slash : backslash;
        return separator ? separator + 1 : path;
    }
}