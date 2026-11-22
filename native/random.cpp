#include "physics_types.hpp"

std::uint32_t native_random_seed=1;
int irand() {
	// Exact 32-bit recurrence in libs/master.lib/random.asm. Unsigned overflow
	// is intentional; do not substitute the host C runtime's rand().
	native_random_seed=native_random_seed*0x015a4e35u+1;
	return (native_random_seed>>16)&0x7fff;
}
