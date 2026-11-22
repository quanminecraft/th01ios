#pragma once
// Turbo C++ uses 16-bit int; shared boss state must retain that width.
#ifdef TH01_NATIVE_ORB_PHYSICS
#include <cstdint>
using yuugenmagan_int_t = std::int16_t;
#else
typedef int yuugenmagan_int_t;
#endif