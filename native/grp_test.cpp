#include "grp.hpp"
#include <iostream>
using namespace th01_native;
static void require(bool condition) { if(!condition) throw std::runtime_error("GRP test failed"); }
static Bytes fixture(const std::string& bits) {
	Bytes file{'Z','N',0,0,0,0,4,0,0,0,0,0,0,0,8,0,1};
	file.resize(17+48);
	file[17+15*3]=15;
	for(std::size_t i=0;i<bits.size();i+=8) {
		unsigned value=0;
		for(unsigned b=0;b<8;++b) value=(value<<1)|((i+b<bits.size() && bits[i+b]=='1') ? 1 : 0);
		file.push_back(value);
	}
	return file;
}
int main() {
	// Seed pair 0/15, horizontal predictor, four pairs (gamma length 4).
	auto file=fixture("1011" "00" "11000");
	auto decoded=read_grp(file);
	require(decoded.palette[15]==0xffff0000 && decoded.images[0].width==8);
	require(decoded.images[0].color==Bytes({0,15,0,15,0,15,0,15}));
	// Copy one pair, repeat predictor => literal pairs. MTF contexts change
	// from 15 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10.
	auto literal=read_grp(fixture("1011" "00" "0" "00" "1011" "1" "1111" "1" "1111"));
	require(literal.images[0].color==Bytes({0,15,15,14,13,12,11,10}));
	unsigned errors=0;
	try {file.resize(64);read_grp(file);} catch(const std::runtime_error&) {++errors;}
	try {auto f=fixture("");read_grp(f);} catch(const std::runtime_error&) {++errors;}
	try {auto f=fixture("1011");f[14]=7;read_grp(f);} catch(const std::runtime_error&) {++errors;}
	try {auto f=fixture("1011");f[17]=16;read_grp(f);} catch(const std::runtime_error&) {++errors;}
	require(errors==4);
	std::cout << "GRP predictor, literal, palette and truncation tests passed\n";
}
