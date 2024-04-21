#pragma once
#ifndef DEF_H
#define DEF_H

#ifdef WIN32
#define FUNCTION __FUNCTION__
#else
#define FUNCTION __PRETTY_FUNCTION__
#endif

#ifdef _MSC_VER
#define TEMP_DISABLE_WARNING(warningCode, expression)   \
        __pragma(warning(push))                             \
        __pragma(warning(disable:warningCode))              \
        expression                                          \
        __pragma(warning(pop))
#else
#define TEMP_DISABLE_WARNING(warningCode, expression)   \
        expression
#endif

// _MSC_VER warning C4127: conditional expression is constant
// use this macro instead "WHILE_FALSE_NO_WARNING", you can enjoy the level 4 warning ^_^
#define WHILE_FALSE_NO_WARNING  \
    TEMP_DISABLE_WARNING(4127, while (false))

#define LOG_PROCESS_ERROR(Condition) \
    do  \
    {   \
        if (!(Condition))       \
        {                       \
            printf(        \
                "LOG_PROCESS_ERROR(%s) at line %d in %s\n", #Condition, __LINE__, FUNCTION  \
            );                  \
            goto Exit0;         \
        }                       \
    } WHILE_FALSE_NO_WARNING

#endif
