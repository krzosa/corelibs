#include "io.h"
#include <stdarg.h>

#ifndef IO_SNPRINTF
    #include <stdio.h>
    #define IO_SNPRINTF snprintf
#endif

#ifndef IO_VSNPRINTF
    #include <stdio.h>
    #define IO_VSNPRINTF vsnprintf
#endif

#ifndef IO_ALLOCATE
    #include <stdlib.h>
    #define IO_ALLOCATE(x) malloc(x)
    #define IO_FREE(x) free(x)
#endif

#ifndef IO_StaticFunc
    #if defined(__GNUC__) || defined(__clang__)
        #define IO_StaticFunc __attribute__((unused)) static
    #else
        #define IO_StaticFunc static
    #endif
#endif

IO_StaticFunc int IO_Strlen(char *string) {
    int len = 0;
    while (*string++ != 0) len++;
    return len;
}

void (*IO_User_OutputMessage)(char *str, int len);

IO_API bool IO__FatalErrorf(const char *file, int line, const char *msg, ...) {
    va_list args1;
    va_list args2;
    char buff[2048];

    va_start(args1, msg);
    va_copy(args2, args1);
    int size = IO_VSNPRINTF(buff, sizeof(buff), msg, args2);
    va_end(args2);

    char *new_buffer = 0;
    char *user_message = buff;
    if (size >= sizeof(buff)) {
        size += 4;
        new_buffer = (char *)IO_ALLOCATE(size);
        IO_VSNPRINTF(new_buffer, size, msg, args1);
        user_message = new_buffer;
    }
    va_end(args1);

    IO_ErrorResult ret = IO_ErrorResult_Continue;
    {
        char buff2[2048];
        char *result = buff2;
        char *b = 0;
        int size = IO_SNPRINTF(buff2, sizeof(buff2), "%s(%d): error: %s \n", file, line, user_message);
        if (size >= sizeof(buff2)) {
            size += 4;
            b = (char *)IO_ALLOCATE(size);
            size = IO_SNPRINTF(b, size, "%s(%d): error: %s \n", file, line, user_message);
            result = b;
        }

        ret = IO_OutputError(result, size);
        if (ret == IO_ErrorResult_Exit) {
            IO_Exit(1);
        }

        if (b) {
            IO_FREE(b);
        }
    }

    if (new_buffer) {
        IO_FREE(new_buffer);
    }

    return ret == IO_ErrorResult_Break;
}

IO_API void IO_Printf(const char *msg, ...) {
    va_list args1;
    va_list args2;
    char buff[2048];

    va_start(args1, msg);
    va_copy(args2, args1);
    int size = IO_VSNPRINTF(buff, sizeof(buff), msg, args2);
    va_end(args2);

    char *new_buffer = 0;
    char *result = buff;
    if (size >= sizeof(buff)) {
        size += 4;
        new_buffer = (char *)IO_ALLOCATE(size);
        IO_VSNPRINTF(new_buffer, size, msg, args1);
        result = new_buffer;
    }
    va_end(args1);

    if (IO_User_OutputMessage) {
        IO_User_OutputMessage(result, size);
    }
    else {
        IO_OutputMessage(result, size);
    }

    if (new_buffer) {
        IO_FREE(new_buffer);
    }
}

IO_API bool IO__FatalError(char *msg) {
    int len = IO_Strlen(msg);
    IO_ErrorResult result = IO_OutputError(msg, len);
    if (result == IO_ErrorResult_Exit) {
        IO_Exit(1);
    }
    return result == IO_ErrorResult_Break;
}

IO_API void IO_Print(char *msg) {
    int len = IO_Strlen(msg);
    if (IO_User_OutputMessage) {
        IO_User_OutputMessage(msg, len);
    }
    else {
        IO_OutputMessage(msg, len);
    }
}
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>

    #pragma comment(lib, "user32")

    #include <stdio.h>

IO_API bool IO_IsDebuggerPresent(void) {
    return IsDebuggerPresent();
}

IO_API void IO_OutputMessage(char *str, int len) {
    if (IsDebuggerPresent()) {
        OutputDebugStringA(str);
    }
    printf("%.*s", len, str);
    fflush(stdout);
}

IO_API IO_ErrorResult IO_OutputError(char *str, int len) {
    IO_ErrorResult result = IO_ErrorResult_Continue;
    IO_OutputMessage(str, len);

    char *msg = str;
    if (str[len] != 0) {
        msg = (char *)IO_ALLOCATE(len + 1);
        for (int i = 0; i < len; i += 1) msg[i] = str[i];
        msg[len] = 0;
    }

    OutputDebugStringA(msg);
    if (!IsDebuggerPresent()) {

        // Limit size of error output message
        char tmp = 0;
        if (len > 4096) {
            tmp = str[4096];
            str[4096] = 0;
        }

        MessageBoxA(0, msg, "Error!", 0);

        if (tmp != 0) {
            str[4096] = tmp;
        }

        result = IO_ErrorResult_Exit;
    }
    else {
        result = IO_ErrorResult_Break;
    }

    if (msg != str) {
        IO_FREE(msg);
    }

    return result;
}

IO_API void IO_Exit(int error_code) {
    ExitProcess(error_code);
}
#else // _WIN32 else // LIBC
    #include <stdio.h>

IO_API IO_ErrorResult IO_OutputError(char *str, int len) {
    fprintf(stderr, "%.*s", len, str);
    return IO_ErrorResult_Exit;
}

IO_API void IO_OutputMessage(char *str, int len) {
    fprintf(stdout, "%.*s", len, str);
}

IO_API void IO_Exit(int error_code) {
    exit(error_code);
}

IO_API bool IO_IsDebuggerPresent(void) {
    return false;
}
#endif // LIBC
