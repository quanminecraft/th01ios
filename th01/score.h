#ifdef TH01_NATIVE_ORB_PHYSICS
#include "native/physics_types.hpp"
#else
#include "platform.h"
#endif

#define SCORE_DIGITS 7

typedef int32_t score_t; // ZUN bug: Should have maybe been unsigned…
typedef uint32_t uscore_t; // ZUN bug: …or consistent, at least.
