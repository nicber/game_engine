#pragma once

#ifdef _MSC_VER
#define no_except throw()
#else
#define no_except noexcept
#endif