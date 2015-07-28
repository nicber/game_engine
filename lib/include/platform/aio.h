#pragma once

#ifdef __unix__
#include "platform/aio/unix.h"
#else
#include "platform/aio/win32.h"
#endif