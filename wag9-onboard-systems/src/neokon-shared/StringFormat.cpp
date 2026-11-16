#include "StringFormat.h"

#include <stdarg.h>

std::string
stringFormatVaList(const char* format, va_list& args)
{
    va_list argsCopy;
    va_copy(argsCopy, args);

    int stringSize = std::vsnprintf(nullptr, 0, format, argsCopy) + 1; // Extra space for '\0'
    va_end(argsCopy);
    if (stringSize <= 0)
        return {};
    std::string result;
    result.reserve(stringSize);

    stringSize = std::vsnprintf(&result[0], stringSize, format, args);
    result.assign(result.data(), stringSize); // Reassign to self data to update length

    return result;
}

std::string
stringFormat(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    std::string result = stringFormatVaList(format, argptr);
    va_end(argptr);
    return result;
}
