#include "pch.h"
#include "logger.h"

#include <stdio.h>

static LogLevel g_logLevel = LogLevel::Info;
constexpr int kMaxLogSize = 2048;

void logger::log(LogLevel level, const char *fmt, ...)
{
    if (g_logLevel > level)
        return;

    char msg[kMaxLogSize] = { 0 };
    va_list vl;
    va_start(vl, fmt);
    _vsnprintf_s(msg, kMaxLogSize, fmt, vl);
    va_end(vl);

    OutputDebugStringA(msg);
}
