#pragma once

#include "stdio.h"

#define LOGGING_VERBOSE 0

namespace LogImpl {
    static const int SPACES_PER_INDENT = 2;

    void Indent();
    void Unindent();
    int GetIndent();

    const char *GetFilename(const char *path);
}

#ifdef _DEBUG

#define LogColourRed(str) "\033[31m" str "\033[0m"
#define LogColourGreen(str) "\033[32m" str "\033[0m"
#define LogColourYellow(str) "\033[33m" str "\033[0m"
#define LogColourBlue(str) "\033[34m" str "\033[0m"
#define LogColourMagenta(str) "\033[35m" str "\033[0m"
#define LogColourCyan(str) "\033[36m" str "\033[0m"

#define LogInfo(format, ...) \
    do { \
        printf(LogColourGreen("[Info]") " "); \
        if (LogImpl::GetIndent() > 0) printf("%*s> ", LogImpl::GetIndent() * LogImpl::SPACES_PER_INDENT, ""); \
        printf(format "\n", ##__VA_ARGS__); \
    } while (0)

#define LogWarn(format, ...) \
        printf(LogColourYellow("[Warn]") " %s:%d in %s(): " format "\n", LogImpl::GetFilename(__FILE__), __LINE__, __func__, ##__VA_ARGS__)

#define LogError(format, ...) \
    do { \
        printf(LogColourRed("[Error]") " %s:%d in %s(): " format "\n", LogImpl::GetFilename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); \
        (void)getchar(); \
    } while (0)

#define LogBlank() printf("\n")

#define LogIndent() LogImpl::Indent()
#define LogUnindent() LogImpl::Unindent()

#else // _DEBUG

#include "Windows.h"

#define LogColourRed(str) (void)0
#define LogColourGreen(str) (void)0
#define LogColourYellow(str) (void)0
#define LogColourBlue(str) (void)0
#define LogColourMagenta(str) (void)0
#define LogColourCyan(str) (void)0

#define LogInfo(format, ...) (void)0

#define LogWarn(format, ...) (void)0

#define LogError(format, ...) \
    do { \
        char buffer[512]; \
        sprintf_s(buffer, "[ERROR] %s:%d in %s(): " format, LogImpl::GetFilename(__FILE__), __LINE__, __func__, ##__VA_ARGS__); \
        wchar_t wideBuffer[512]; \
        size_t dummy = 0; \
        mbstowcs_s(&dummy, wideBuffer, buffer, _TRUNCATE); \
        MessageBox(NULL, wideBuffer, L"Fatal error", MB_OK | MB_ICONERROR); \
    } while (0)

#define LogBlank() (void)0

#define LogIndent() (void)0
#define LogUnindent() (void)0

#endif // _DEBUG