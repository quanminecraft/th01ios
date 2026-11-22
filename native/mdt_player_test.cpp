#include "mdt_player.hpp"
#include "assets.hpp"
#include <iostream>
int main(int argc,char** argv) {try {
 using namespace th01_native;
 if(argc!=2)throw std::invalid_argument("Usage: mdt_player_test ASSET_DIR");
 unsigned files=0;
 for(auto& item:std::filesystem::directory_iterator(argv[1])) {
  if(item.path().extension()!=".MDT")continue;
  std::uint64_t writes=0;unsigned drums=0;
  try {
   MdtPlayer song(read_file(item.path()),[&](unsigned reg,unsigned value) {if(reg>511||value>255)throw std::runtime_error("Invalid OPNA register write");++writes;},[&](unsigned mask){if(mask>63)throw std::runtime_error("Invalid rhythm mask");++drums;});
   for(unsigned i=0;i<15000&&!song.finished();++i)song.tick();
   if(item.path().stem()!="INIT"&&(!song.notes()||!song.loops()))throw std::runtime_error("Song did not reach notes and loop points");
   std::cout<<item.path().filename().string()<<" notes="<<song.notes()<<" loops="<<song.loops()<<" writes="<<writes<<" drums="<<drums<<'\n';++files;
  }catch(const std::exception& e) {throw std::runtime_error(item.path().filename().string()+": "+e.what());}
 }
 if(files<15)throw std::runtime_error("Missing original music data");
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}
