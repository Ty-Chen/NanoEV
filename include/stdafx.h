#pragma once
#pragma once

#ifndef _STD_AFX_H_
#define _STD_AFX_H_

#ifdef WIN32
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#pragma warning(disable:4127)

#pragma warning(disable:4200)
#pragma warning(disable:4201)

#pragma warning(disable:4819)
#endif

#include <iostream>
#include <conio.h>
#include <atlbase.h>
#include <atltrace.h>
#include <cassert>
#include <signal.h>
#include <list>
#include <vector>
#include <atomic>


#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#include <WinCon.h>
#include <tchar.h>
#include <conio.h>
#endif

#include <string.h>
#include <locale.h>

#ifdef __GNUC__
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <limits.h>
#endif

#endif  //_STD_AFX_H_
