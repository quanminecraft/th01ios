#include "defeat_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: defeat_test ASSET_DIR [BMP_PREFIX]");
		auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();FontRom font(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));
		auto stg=read_sprites(asset(archive,"STG.PTN")),miko=read_sprites(asset(archive,"MIKO.PTN")),kuji1=read_sprites(asset(archive,"KUZI1.GRC")),kuji2=read_sprites(asset(archive,"KUZI2.GRC"));
		auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS1.GRP"));fb->palette=bg.palette;for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		BombBackend bomb{*fb,kuji1,kuji2,[] {}};BombBackendBinding bomb_binding(bomb,player);unsigned waits=0,gap=0;bool save_default=true;
		SingyokuBackend singyoku{[&](unsigned n) {waits+=n;if(n==50) {++gap;if(argc==3 && save_default) dump_bmp(std::string(argv[2])+"-makai.bmp",*fb);}},[] {},[] {},bomb.current};SingyokuBinding singyoku_binding(singyoku);
		route_t route=ROUTE_MAKAI;unsigned polls=0;bool choose_jigoku=true;
		DefeatBackend defeat{*fb,font,bomb.stage,route,[&]() {InputKeys keys;unsigned n=polls++;if(choose_jigoku) {keys.down=n==2 || n==3;keys.enter=n==5;}else keys.shot=n==1;if(n>10) throw std::runtime_error("Route input loop did not acknowledge");return keys;}};DefeatBinding binding(defeat);
		boss_entity_0.pos_set(272,96);player_left=304;page_access(0);stage_cleared=false;player_is_hit=false;
		singyoku_defeat_animate_and_select_route();
		require(route==ROUTE_JIGOKU && stage_cleared && player_is_hit && polls==6,"Original route toggles once while down is held, confirms Enter and sets clear/hit");
		require(gap==1 && waits>500 && native_bomb_palette()[3].c.r==15 && native_bomb_palette()[1].c.r==9,"Original fade, 50-frame menu gap and selected route palette");
		require(!shots.sound_commands.empty() && shots.sound_commands.back()==4,"Route acknowledgement emits original SE 4");
		if(argc==3) dump_bmp(std::string(argv[2])+"-jigoku.bmp",*fb);
		choose_jigoku=false;save_default=false;polls=0;stage_cleared=false;player_is_hit=false;
		singyoku_defeat_animate_and_select_route();require(route==ROUTE_MAKAI && polls==2 && stage_cleared,"New route menu defaults to Makai and accepts shot");
		std::cout<<"Original boss defeat fade, doubled route text, held-input toggle and both routes passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
