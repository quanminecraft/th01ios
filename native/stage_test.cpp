#include "stage.hpp"
#include <iostream>
using namespace th01_native;
static void require(bool condition) { if(!condition) throw std::runtime_error("Stage test failed"); }
int main() {
	Bytes file(1272);std::copy_n("STAGE",5,file.begin());
	file[22]=9; // cards at columns 0 and 3: high-to-low low-nibble bits
	file[22+50]=16;file[22+50+3]=14;
	file[22+50+20+19]=17; // portal at (608,96)
	file[22+250+4]=1; // next stage: column 19
	auto scene=read_scene(file);
	require(scene[0].cards.size()==2 && scene[0].cards[0].left==0 && scene[0].cards[1].left==96);
	require(scene[0].cards[0].hp==3 && scene[0].cards[1].hp==1);
	require(scene[0].obstacles.size()==1 && scene[0].obstacles[0].left==608 && scene[0].obstacles[0].top==96);
	require(scene[1].cards.size()==1 && scene[1].cards[0].left==608 && scene[2].cards.empty());
	unsigned rejected=0;
	try {file[0]=0;read_scene(file);} catch(const std::runtime_error&) {++rejected;}
	try {file.resize(1271);read_scene(file);} catch(const std::runtime_error&) {++rejected;}
	require(rejected==2);
	std::cout << "Stage card order, HP, coordinates and bounds tests passed\n";
}
