#include "bonus_values.hpp"
#include <iostream>
#include <stdexcept>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		auto first=stage_bonus_values(700,5,4,3,1);
		require(first.metrics==std::array<std::int16_t,4>{2100,500,1100,0} && first.total==37000,"Stage 1 original four metrics and x10 total");
		auto later=stage_bonus_values(3000,100,4,3,4);
		require(later.metrics==std::array<std::int16_t,4>{6553,6553,1100,600} && later.total==148060,"Upper clamps precede accumulation; stage uses stage minus one");
		auto overflow=stage_bonus_values(12000,0,0,0,1);
		require(overflow.metrics[0]==-29536 && overflow.total==4294671936u,"DOS signed assignment wraps before clamp; unsigned bonus accumulator preserves quirk");
		require(stage_bonus_values(0,0,0,0,1).total==0,"Empty metrics grant no points");
		for(unsigned raw=0;raw<65536;++raw) {
			int signed_value=raw<32768 ? int(raw) : int(raw)-65536;
			require(bonus_dos_int(signed_value)==signed_value && bonus_dos_int(signed_value+65536)==signed_value,"DOS narrowing is independent of host int width");
		}
		std::cout<<"Original regular-stage bonus metrics, caps and DOS-width arithmetic passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
