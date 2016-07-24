#pragma once

#ifndef _WIN32
#include "stack_allocator_linux.h"
#else
#include "stack_allocator_win32.h"
#endif