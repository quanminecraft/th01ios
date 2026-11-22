#include "math_backend.hpp"
#include "th01/math/polar.hpp"
#include "th01/math/vector.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main() {
	try {
		require(iatan2(0,0)==0 && iatan2(0,1)==0 && iatan2(1,0)==64 && iatan2(0,-1)==128 && iatan2(-1,0)==192,"Angle axes");
		require(iatan2(1,1)==32 && iatan2(1,-1)==96 && iatan2(-1,-1)==160 && iatan2(-1,1)==224,"Angle quadrants");
		for(int angle=0;angle<256;++angle) require(SinTable8[angle]==CosTable8[(angle+192)&255],"Original table phase relation");
		std::int16_t x,y;vector2(x,y,56,32);require(x==39 && y==39,"Positive diagonal");
		vector2(x,y,56,160);require(x==-40 && y==-40,"Negative polar shift asymmetry");
		vector2_between(100,100,200,200,x,y,56);require(x==39 && y==39,"Aimed diagonal");
		vector2_between(200,200,100,100,x,y,56);require(x==-40 && y==-40,"Negative aimed diagonal");
		for(int dy=-640;dy<=640;dy+=17) for(int dx=-640;dx<=640;dx+=17) {
			if(!dx && !dy) continue;
			double exact=std::atan2(double(dy),double(dx))*128/std::acos(-1.0);
			if(exact<0) exact+=256;
			double delta=std::abs(iatan2(dy,dx)-exact);delta=std::min(delta,256-delta);
			require(delta<1.0,"Integer lookup angle deviates by a full angle unit");
		}
		bool rejected=false;try {iatan2(-32768,0);} catch(const std::domain_error&) {rejected=true;}
		require(rejected,"Original signed-absolute DIV boundary");
		std::uint64_t hash=14695981039346656037ull;
		for(int dy=-400;dy<=400;++dy) for(int dx=-640;dx<=640;++dx) {hash^=iatan2(dy,dx);hash*=1099511628211ull;}
		std::cout << "Original integer aim tables/vector math passed; angle-grid trace " << std::hex << hash << '\n';
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
