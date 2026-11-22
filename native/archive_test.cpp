#include "archive.hpp"
#include <iostream>
using namespace th01_native;
static Bytes encode(Bytes b) { for(auto& c : b) c ^= 0x76; return b; }
static void require(bool condition) { if(!condition) throw std::runtime_error("Test failed"); }
int main() {
	require(decode(encode({'z','z',2,'z',1,'y','y',0}),8,true) == Bytes({'z','z','z','z','z','z','y','y'}));
	require(decode(encode({'a','b','c'}),3,false) == Bytes({'a','b','c'}));
	require(decode(encode({'a'}),1,true) == Bytes({'a'}));
	require(decode({},0,true).empty());
	unsigned rejected = 0;
	try { decode(encode({'a'}),2,true); } catch(const std::runtime_error&) { ++rejected; }
	try { decode({},1,false); } catch(const std::runtime_error&) { ++rejected; }
	try { read_archive(Bytes(20)); } catch(const std::runtime_error&) { ++rejected; }
	Bytes bad(2048); bad[0]=1;
	try { read_archive(bad); } catch(const std::runtime_error&) { ++rejected; }
	require(rejected == 4);
	// Real TH01 uses a compact directory, although DOS reads 64 slots.
	Bytes compact(2048);
	compact[0]=0x88; compact[1]=0xf3; compact[2]=3;
	compact[3]='A'^0xff;
	compact[16]=1; compact[20]=1; compact[24]=64;
	compact[64]='q'^0x76;
	auto entries = read_archive(compact);
	require(entries.size()==1 && entries[0].name=="A" && entries[0].data==Bytes({'q'}));
	std::cout << "Archive tests passed\n";
}
