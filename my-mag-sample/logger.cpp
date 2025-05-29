#include "stdafx.h"
#include "logger.h"
#include <iostream>

static logger::LogLevel g_logLevel = logger::LogLevel::Info;
const int kMaxLogSize = 2048;
static const char *logPrefix[] = {
    "Trace", "Debug", "Info", "Warn", "Error", "Assert", "Release",
};

static void _log(logger::LogLevel level, const char *fmt, va_list vl)
{
    if (g_logLevel > level)
        return;

    char msg[kMaxLogSize] = { 0 };
    _vsnprintf_s(msg, kMaxLogSize, fmt, vl);

    std::cout << logPrefix[level] << " - " << msg << std::endl;
}

void logger::log(LogLevel level, const char *fmt, ...)
{
    if (g_logLevel > level)
        return;

    char msg[kMaxLogSize] = { 0 };
    va_list vl;
    va_start(vl, fmt);
    _vsnprintf_s(msg, kMaxLogSize, fmt, vl);
    va_end(vl);

    std::cout << logPrefix[level] << " - " << msg << std::endl;
}

void logger::logInfo(const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    _log(LogLevel::Info, fmt, vl);
    va_end(vl);
}

void logger::logError( const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    _log(LogLevel::Error, fmt, vl);
    va_end(vl);
}

void logger::logWarning(const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    _log(LogLevel::Warn, fmt, vl);
    va_end(vl);
}
