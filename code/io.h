#pragma once
#include <stdbool.h>

#ifndef IO_API
    #ifdef __cplusplus
        #define IO_API extern "C"
    #else
        #define IO_API
    #endif
#endif

typedef enum IO_ErrorResult {
    IO_ErrorResult_Continue,
    IO_ErrorResult_Break,
    IO_ErrorResult_Exit,
} IO_ErrorResult;

#ifdef _WIN32
    #define IO_DebugBreak() (__debugbreak(), 0)
#else
    #define IO_DebugBreak() (IO_Exit(1), 0)
#endif

extern void (*IO_User_OutputMessage)(char *str, int len);

#define IO__STRINGIFY(x) #x
#define IO__TOSTRING(x) IO__STRINGIFY(x)
#define IO_LINE IO__TOSTRING(__LINE__)

#define IO_Assert(x) !(x) && IO__FatalError((char *)(__FILE__ "(" IO_LINE "): "      \
                                                              "error: " #x "\n")) && \
                         IO_DebugBreak()
#define IO_FatalErrorf(...)                                             \
    do {                                                                \
        bool result = IO__FatalErrorf(__FILE__, __LINE__, __VA_ARGS__); \
        if (result) IO_DebugBreak();                                    \
    } while (0)
#define IO_FatalError(...)                                                            \
    do {                                                                              \
        bool result = IO__FatalError(__FILE__ "(" IO_LINE "): error - " __VA_ARGS__); \
        if (result) IO_DebugBreak();                                                  \
    } while (0)
#define IO_Assertf(x, ...)                                                  \
    do {                                                                    \
        if (!(x)) {                                                         \
            bool result = IO__FatalErrorf(__FILE__, __LINE__, __VA_ARGS__); \
            if (result) IO_DebugBreak();                                    \
        }                                                                   \
    } while (0)

#define IO_InvalidElseIf(c)   \
    else if (c) {             \
        IO_InvalidCodepath(); \
    }
#define IO_InvalidElse()      \
    else {                    \
        IO_InvalidCodepath(); \
    }
#define IO_InvalidCodepath() IO_FatalError("This codepath is invalid")
#define IO_InvalidDefaultCase()                                 \
    default: {                                                  \
        IO_FatalError("Entered invalid switch statement case"); \
    }
#define IO_Todo() IO_FatalError("This codepath is not implemented yet")

IO_API bool IO__FatalErrorf(const char *file, int line, const char *msg, ...);
IO_API void IO_Printf(const char *msg, ...);
IO_API bool IO__FatalError(char *msg);
IO_API void IO_Print(char *msg);
IO_API void IO_OutputMessage(char *str, int len);
IO_API IO_ErrorResult IO_OutputError(char *str, int len);
IO_API void IO_Exit(int error_code);
IO_API bool IO_IsDebuggerPresent(void);
