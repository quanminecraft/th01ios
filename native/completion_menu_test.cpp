#include "title_menu.hpp"
#include "boss_practice.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <cctype>
#include <iostream>
using namespace th01_native;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
 if(argc!=3)throw std::invalid_argument("Usage: completion_menu_test ASSETS TEST_PROFILE");
 std::filesystem::path assets=argv[1],profile=argv[2];std::filesystem::create_directories(profile);std::filesystem::remove(profile/"OTHER.DAT");
 BossProgress p(profile);require(!p.has_completion(),"Fresh save hides Boss Prac");
 require(p.defeated(GameMode::standard,1,BossId::mima)&&!p.has_completion(),"A midboss clear cannot open practice menu");
 FontRom font(read_file(assets/"FONT.ROM"));
 TitleHost host{[&](const char* file){std::string n=file;for(char& c:n)c=std::toupper(static_cast<unsigned char>(c));return read_grp(read_file(assets/n));},[](const Framebuffer&,unsigned){},[](unsigned){},[](const char*,const char*){}};
 auto choose_third=[&](TitleMenu& t){InputKeys k;k.shot=true;t.step(k);t.step({});k={};k.down=true;for(int i=0;i<2;++i){t.step(k);t.step({});}k={};k.shot=true;return t.step(k);};
 {TitleMenu fresh(font,profile,host);require(choose_third(fresh)==TitleAction::menu&&fresh.menu_id()==1,"Third menu entry is Option before completion");dump_bmp(profile/"fresh-options.bmp",fresh.framebuffer());}
 require(p.defeated(GameMode::challenge,3,BossId::sariel)&&p.has_completion(),"Any final boss/route/mode/rank enables the main-menu feature");
 require(p.completed(GameMode::challenge,3)&&!p.completed(GameMode::standard,1),"Completion flag does not leak to other groups");
 BossProgress reopened(profile);require(reopened.has_completion()&&!reopened.unlocked(GameMode::standard,1,BossId::sariel),"Global menu gate never grants another group's boss unlock");
 {TitleMenu done(font,profile,host);require(choose_third(done)==TitleAction::boss_practice,"Third entry becomes Boss Prac after completion/relaunch");}
 auto bytes=read_file(profile/"OTHER.DAT");require(bytes.size()==24&&bytes[8]==2,"Version2 persists the completion flags separately from seven unlock bits");
 // Version1 final-boss unlock was only granted by an actual normal-session clear.
 auto legacy=profile/"legacy";std::filesystem::create_directories(legacy);bytes[8]=1;for(unsigned i=12;i<20;++i)bytes[i]&=127;std::uint32_t sum=2166136261u;for(unsigned i=0;i<20;++i){sum^=bytes[i];sum*=16777619u;}for(unsigned i=0;i<4;++i)bytes[20+i]=sum>>(8*i);
 {std::ofstream out(legacy/"OTHER.DAT",std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());}
 BossProgress old(legacy);require(old.has_completion()&&old.unlocked(GameMode::standard,1,BossId::mima),"Existing v1 final-clear progress migrates without losing boss unlocks");require(old.defeated(GameMode::standard,0,BossId::shingyoku),"Migrated progress can be atomically replaced");require(read_file(legacy/"OTHER.DAT")[8]==2,"Migrated writes use version2");
 std::cout<<"Fresh/clear-gated menu, independent boss groups, reopen and v1 migration passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
