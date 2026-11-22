#include "regist_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <cstring>
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<3 || argc>4) throw std::invalid_argument("Usage: regist_test ASSET_DIR FIXTURE_DIRECTORY [BMP_PREFIX]");
		auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();FontRom font(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));
		auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN")),kuji1=read_sprites(asset(archive,"KUZI1.GRC")),kuji2=read_sprites(asset(archive,"KUZI2.GRC"));
		auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"GAME_O.GRP"));fb->palette=bg.palette;
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		BombBackend bomb{*fb,kuji1,kuji2,[] {}};BombBackendBinding bomb_binding(bomb,player);CardRenderer renderer{*fb,stg,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		ScoredatBackend files{argv[2],{}};ScoredatBinding files_binding(files);rank=RANK_NORMAL;scoredat_recreate();
		unsigned polls=0,delays=0,presents=0;int mode=0;
		RegistBackend regist{[&]() {unsigned n=polls++;if(n>30) throw std::runtime_error("Registration input did not terminate");InputKeys keys;
			if(mode==0) {keys.shot=n==2 || n==6;keys.right=n==4;keys.enter=n==8;}
			if(mode==1) keys.enter=n==2;
			if(mode==2) keys.shot=n>=2 && n%2==0;
			if(mode==3) keys.shot=n==1;
			return keys;},[&](unsigned n) {require(n==4,"Original registration delays four frames per processed input");delays+=n;},[&]() {++presents;if(argc==4 && mode==0 && (polls==2 || polls==8)) dump_bmp(std::string(argv[3])+(polls==2 ? "-initial.bmp" : "-entered.bmp"),*fb);}};RegistBinding regist_binding(regist);
		auto reset=[&]() {for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);page_access(0);z_palette_set_show(3,7,15,7);z_palette_set_show(7,15,15,15);polls=delays=presents=0;};
		reset();regist_menu(5000,5,0xd08e);
		require(polls==9 && delays==12 && presents>=9,"Name input accepts shot, moves right and confirms Enter");
		require(scoredat_names==nullptr && scoredat_score==nullptr,"Registration frees original score buffers on return");
		require(scoredat_load()==0 && scoredat_score[0]==5000 && scoredat_score[1]==1000 && scoredat_stages[0]==5 && scoredat_routes[0]==0xd08e,"New high score shifts complete table and stores stage/route");
		unsigned char name[17];scoredat_name_get(0,name);require(name[0]==0x82 && name[1]==0x81 && name[2]==0x82 && name[3]==0x82,"Original alphabet inserts fullwidth a/b bytes");scoredat_free();
		mode=1;reset();regist_menu(5000,4,0xd08e);require(scoredat_load()==0 && scoredat_score[0]==5000 && scoredat_score[1]==5000 && scoredat_stages[0]==4 && scoredat_stages[1]==5,"Equal score ranks ahead of prior entry");scoredat_free();
		mode=2;reset();regist_menu(6000,3,0xd08e);require(polls==19 && delays==32,"Eight entered glyphs jump to End; next shot confirms");
		require(scoredat_load()==0,"Reload eight-glyph name");scoredat_name_get(0,name);for(unsigned i=0;i<16;i+=2) require(name[i]==0x82 && name[i+1]==0x81,"All eight fullwidth glyphs persist");scoredat_free();
		auto before=read_file(files.directory/"REYHINO.DAT");mode=3;reset();regist_menu(0,1,0xd08e);
		require(polls==3 && delays==0 && read_file(files.directory/"REYHINO.DAT")==before,"Nonqualifying result waits for press/release without modifying score file");
		std::cout<<"Original registration table, name input, ties, eight-glyph End and nonqualifying result passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
