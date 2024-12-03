#ifndef __LOG_H__
#define __LOG_H__

#include<stdio.h>

#ifdef _DEBUG_
#define DEBUG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#endif