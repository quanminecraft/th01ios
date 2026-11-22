#include "mdt_player.hpp"
#include "assets.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
int main(int argc,char** argv){try{
 using namespace th01_native;
 if(argc!=4)throw std::runtime_error("Usage: mdt_trace MDT OUTPUT.csv SECONDS");
 std::ofstream out(argv[2]);if(!out)throw std::runtime_error("Cannot open trace output");
 unsigned tick=0;double time=0,limit=std::stod(argv[3]);out<<"tick,seconds,reg,value\n"<<std::setprecision(12);
 auto log=[&](unsigned reg,unsigned value){out<<tick<<','<<time<<','<<reg<<','<<value<<'\n';};
 MdtPlayer player(read_file(argv[1]),log,[&](unsigned mask){log(0x10,mask);});
 while(time<limit&&!player.finished()){player.tick();time+=player.timer_period()*48.0/166400.0;++tick;}
 std::cout<<tick<<" ticks, "<<player.notes()<<" notes; nominal seconds="<<time<<'\n';
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
