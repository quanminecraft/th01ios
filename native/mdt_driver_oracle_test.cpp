#include "mdt_player.hpp"
#include "assets.hpp"
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
using namespace th01_native;
static std::vector<std::vector<unsigned>> csv(const std::filesystem::path& p){
 std::ifstream f(p);if(!f)throw std::runtime_error("Missing driver oracle: "+p.string());std::string line;std::getline(f,line);std::vector<std::vector<unsigned>> rows;
 while(std::getline(f,line)){std::stringstream stream(line);std::string value;std::vector<unsigned> row;while(std::getline(stream,value,','))row.push_back(std::stoul(value));rows.push_back(row);}return rows;
}
static void check_gates(const std::filesystem::path& assets,const std::filesystem::path& fixtures,const std::string& song,const std::string& fixture){
 auto expected=csv(fixtures/fixture);unsigned mask=0;
 MdtPlayer player(read_file(assets/song),[&](unsigned reg,unsigned value){if(reg!=40||(value&3)==3)return;unsigned ch=(value&3)+((value&4)?3:0);if(value&240)mask|=1u<<ch;else mask&=~(1u<<ch);},[](unsigned){});
 for(auto& row:expected){player.tick();if(mask!=row.at(1))throw std::runtime_error(song+" original gate differs at tick "+std::to_string(row.at(0)));}
 std::cout<<song<<": "<<expected.size()<<" gate ticks match original driver\n";
}
int main(int argc,char** argv){try{
 if(argc!=3)throw std::runtime_error("Usage: mdt_driver_oracle_test ASSETS FIXTURES");
 auto fm=csv(std::filesystem::path(argv[2])/"mdrv-reimu-fm.csv"),ssg=csv(std::filesystem::path(argv[2])/"mdrv-reimu-ssg.csv");
 std::map<std::pair<unsigned,unsigned>,std::vector<unsigned>> expected;for(auto& row:fm)expected[{row.at(0),row.at(1)}]=row;
 auto gates=csv(std::filesystem::path(argv[2])/"mdrv-reimu-gates.csv");
 std::array<unsigned,512> state{};unsigned tick=0,compared=0,keymask=0;
 MdtPlayer player(read_file(std::filesystem::path(argv[1])/"REIMU.MDT"),[&](unsigned reg,unsigned value){
  state.at(reg)=value;
  if(reg==0x28&&(value&3)!=3){unsigned ch=(value&3)+((value&4)?3:0);if(value&240)keymask|=1u<<ch;else keymask&=~(1u<<ch);}
  if(reg!=0x28||!(value&240))return;
  unsigned ch=(value&3)+((value&4)?3:0),base=ch%3+(ch>=3?256:0);
  std::vector<unsigned> actual{tick,ch,((state[base+0xa4]&63)<<8)|state[base+0xa0],state[base+0xb0]&63};
  for(unsigned op=0;op<4;++op)actual.push_back(state[base+0x40+op*4]);
  for(unsigned group:{0x30,0x50,0x60,0x70,0x80})for(unsigned op=0;op<4;++op)actual.push_back(state[base+group+op*4]);
  auto it=expected.find({tick,ch});if(it==expected.end()||it->second!=actual)throw std::runtime_error("Original driver FM differs at tick "+std::to_string(tick)+", channel "+std::to_string(ch));++compared;
 },[](unsigned){});
 for(auto& row:ssg){tick=row.at(0);player.tick();if(keymask!=gates.at(tick).at(1))throw std::runtime_error("Original FM key state differs at tick "+std::to_string(tick));std::vector<unsigned> actual{tick};for(unsigned r=0;r<7;++r)actual.push_back(state[r]);for(unsigned r=8;r<=10;++r)actual.push_back(state[r]);actual.push_back(state[17]);if(actual!=row)throw std::runtime_error("Original driver SSG/rhythm differs at tick "+std::to_string(tick));}
 if(compared!=fm.size()||compared<100||ssg.size()!=476)throw std::runtime_error("Incomplete driver comparison");
 check_gates(argv[1],argv[2],"ST0.MDT","mdrv-st0-gates.csv");
 check_gates(argv[1],argv[2],"LEGEND.MDT","mdrv-legend-gates.csv");
 std::cout<<compared<<" FM note states and "<<ssg.size()<<" SSG/rhythm/gate ticks match MDRV98\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
