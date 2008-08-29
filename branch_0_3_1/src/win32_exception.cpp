

#include "stdAfx.hpp"

#include "win32_exception.hpp"
#include "eh.h"

void win32_exception::install_handler()
{
    _set_se_translator(win32_exception::translate);
}

void win32_exception::translate(unsigned code, EXCEPTION_POINTERS* info)
{
    // Windows guarantees that *(info->ExceptionRecord) is valid
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
        throw access_violation(*(info->ExceptionRecord));
        break;
    default:
        throw win32_exception(*(info->ExceptionRecord));
    }
}

win32_exception::win32_exception(const EXCEPTION_RECORD& info)
: mWhat("Win32 exception"), mWhere(info.ExceptionAddress), mCode(info.ExceptionCode)
{
    switch (info.ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        mWhat = "Access violation";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        mWhat = "Division by zero";
        break;
    }
}

access_violation::access_violation(const EXCEPTION_RECORD& info)
: win32_exception(info), mIsWrite(false), mBadAddress(0)
{
    mIsWrite = info.ExceptionInformation[0] == 1;
    mBadAddress = reinterpret_cast<win32_exception::Address>(info.ExceptionInformation[1]);
}
