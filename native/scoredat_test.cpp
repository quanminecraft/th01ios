#include "scoredat_backend.hpp"
#include "assets.hpp"
#include <cstring>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc!=2) throw std::invalid_argument("Usage: scoredat_test FIXTURE_DIRECTORY");
		ScoredatBackend files{argv[1],{}};ScoredatBinding binding(files);
		const char* names[]={"REYHIES.DAT","REYHINO.DAT","REYHIHA.DAT","REYHILU.DAT"};
		for(unsigned value=0;value<256;++value) {auto byte=static_cast<std::int8_t>(value);require(scoredat_name_byte_decode(scoredat_name_byte_encode(byte))==byte,"Original name cipher wraps and roundtrips all byte values");}
		Bytes initial;
		for(rank=0;rank<4;++rank) {
			scoredat_recreate();auto disk=read_file(files.directory/names[rank]);
			require(disk.size()==247 && std::string(disk.begin(),disk.begin()+7)=="HISCORE" && disk[7]==0x2f && disk[8]==0x28,"Original 247-byte layout, magic and encoded first kanji");
			require(disk[167]==0xe8 && disk[168]==3 && disk[207]==10 && disk[227]==0x96 && disk[228]==0xb3,"Little-endian score/stage and original no-route kanji fields");
			if(initial.empty()) initial=disk;else require(initial==disk,"All four recreated difficulty files use the original defaults");
			require(scoredat_load()==0 && scoredat_hiscore_get()==1000,"Original load decodes recreated rank file");
			unsigned char name[17];scoredat_name_get(0,name);require(name[0]==0x93 && name[1]==0x8c && name[16]==0,"Default Shift-JIS name is decoded and null-terminated for UI");
			for(int i=0;i<10;++i) require(scoredat_score[i]==1000-i*100 && scoredat_stages[i]==10-i,"Default rank scores/stages descend exactly as source");
			scoredat_free();
		}
		rank=1;require(scoredat_load()==0,"Load Normal for roundtrip");
		scoredat_score[0]=7654321;scoredat_stages[0]=-123;scoredat_routes[0]=0x8296;unsigned char entered[17]="Native score    ";
		std::memcpy(scoredat_names,entered,16);scoredat_save();
		require(static_cast<unsigned char>(scoredat_names[0])==static_cast<unsigned char>('N'+0x9c),"Original save mutates decoded name storage to encoded bytes");
		scoredat_free();require(scoredat_load()==0 && scoredat_score[0]==7654321 && scoredat_stages[0]==-123 && scoredat_routes[0]==0x8296,"Score, signed stage and route survive disk roundtrip");
		unsigned char loaded[17];scoredat_name_get(0,loaded);require(std::memcmp(loaded,entered,17)==0,"Entered 16-byte name roundtrips");scoredat_free();
		auto disk=read_file(files.directory/names[1]);disk[4]='x';disk[5]='y';disk[6]='z';
		auto write=[&](const Bytes& bytes) {std::ofstream out(files.directory/names[1],std::ios::binary|std::ios::trunc);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());if(!out) throw std::runtime_error("Cannot write fixture");};
		write(disk);require(scoredat_load()==0,"Original validator intentionally checks only first four magic bytes");scoredat_free();
		disk[0]='X';write(disk);require(scoredat_load()==1,"Invalid first four magic bytes are rejected");
		disk.resize(20);write(disk);require(scoredat_load()==1,"Native I/O rejects short files before original allocation/read path");
		scoredat_recreate();std::cout<<"Original score file creation, all ranks, cipher, disk roundtrip and validation passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
