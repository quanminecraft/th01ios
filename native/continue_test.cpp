#include "continue_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: continue_test ASSET_DIR [BMP_PREFIX]");
		auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();FontRom font(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));
		auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN")),kuji1=read_sprites(asset(archive,"KUZI1.GRC")),kuji2=read_sprites(asset(archive,"KUZI2.GRC"));
		auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"GAME_O.GRP"));
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		BombBackend bomb{*fb,kuji1,kuji2,[] {}};BombBackendBinding bomb_binding(bomb,player);
		ContinueResident resident;std::uint16_t continues=0;unsigned polls=0,waits=0,stops=0;int mode=0;
		ContinueBackend menu{*fb,font,resident,continues,[&]() {InputKeys keys;unsigned n=polls++;if(mode==5) {keys.enter=n<5;keys.down=n==6;keys.shot=n==8;return keys;}if(n==0) return keys;if(mode==0) keys.enter=true;if(mode==1) {keys.down=n==1;keys.shot=n==2;}if(mode==2) keys.escape=true;if(mode==4) {keys.enter=true;keys.down=true;}return keys;},[&](unsigned n) {require(n==1,"Continue waits one frame per iteration");++waits;if(argc==3 && ((mode==0 && waits==1)||(mode==1 && waits==2))) dump_bmp(std::string(argv[2])+(mode==0 ? "-yes.bmp" : "-no.bmp"),*fb);},[&] {++stops;}};
		ContinueBinding binding(menu);
		auto reset=[&]() {fb->palette=bg.palette;for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);page_access(0);polls=waits=0;score=123450;resident={};resident.score=score;resident.point_value=1000;native_pellet_speed=10;player_is_hit=true;};
		reset();require(continue_menu() && waits==1 && resident.point_value==0 && !resident.snd_need_init,"Default Yes requests reload and clears point value/audio-init flag");
		require(continues==1 && resident.score==0 && resident.rem_lives==4 && resident.score_highest==score && native_pellet_speed==-2,"Original menu-entry resident updates and integer pellet speed clamp");
		mode=1;reset();require(!continue_menu() && waits==2 && stops==1 && !player_is_hit && resident.point_value==1000,"Down selects No; confirm clears hit/stops music without resetting point value");
		mode=2;reset();require(!continue_menu() && waits==1 && stops==1 && !paused && !player_is_hit,"Escape returns without the confirm/timeout music-stop branch");
		mode=3;reset();require(!continue_menu() && waits==3001 && stops==2,"Original timeout is strictly beyond 3000 frames");
		mode=4;reset();native_pellet_speed=-4;require(continue_menu() && native_pellet_speed==-9,"Confirm is processed before same-frame Down and existing slow pellets lower by five units");
		require(continues==5,"Menu entry consumes a continue count even for No, Escape and timeout");
        mode=5;reset();require(!continue_menu() && waits==8,"Held registration confirm is consumed until release; fresh Down/confirm chooses No");
		std::cout<<"Original Continue Yes/No, Escape, timeout, input order and resident state passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
