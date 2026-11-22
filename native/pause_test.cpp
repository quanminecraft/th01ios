#include "pause_backend.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
static SpriteSet bank(unsigned count) {SpriteSet set;for(unsigned i=0;i<count;++i) set.images.push_back({32,32,Bytes(1024,2),Bytes(1024,1)});return set;}
int main(int argc,char** argv) {
	try {
		if(argc>3) throw std::invalid_argument("Usage: pause_test [ASSET_DIR [BMP_PREFIX]]");
		auto fb=std::make_unique<Framebuffer>();auto stg=bank(34),miko=bank(28),kuji1=bank(8),kuji2=bank(1);FontRom font(Bytes(0x46800));
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);PlayerBackend player{shots,miko,miko,miko,{},{},{}};PlayerBackendBinding player_binding(player);
		CardRenderer renderer{*fb,stg,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);
		for(unsigned i=0;i<16;++i) fb->palette[i]=0xff000000u|i*0x111111u;
		BombBackend bomb{*fb,kuji1,kuji2,[]() {}};BombBackendBinding bomb_binding(bomb,player);
		std::uint16_t continues=0;std::vector<InputKeys> script;std::vector<unsigned> delays;unsigned index=0;bool real=false;
		PauseBackend backend{{},[&](unsigned n) {delays.push_back(n);},continues};
		backend.poll=[&]() {
			if(index>=script.size()) throw std::runtime_error("Pause input script exhausted");
			if(real && argc==3 && index<2) dump_bmp(std::string(argv[2])+(index==0 ? "-pause.bmp" : "-confirm.bmp"),*fb);
			return script[index++];
		};PauseBackendBinding pause_binding(backend);
		auto reset=[&]() {
			input_reset_sense();player_is_hit=false;bomb_doubletap_frame=200;continues=0;index=0;delays.clear();
			fb->clear(0,3);fb->clear(1,3);page_access(0);z_palette_set_all_show(native_bomb_stage_palette());
			InputKeys escape;escape.escape=true;set_input_keys(escape);input_sense(false);require(paused,"Game Escape press enters pause state");
		};
		InputKeys enter;enter.enter=true;InputKeys right_enter=enter;right_enter.right=true;InputKeys escape;escape.escape=true;
		reset();auto original=fb->palette;script={enter};
		require(!pause_menu() && !paused && continues==0 && fb->palette==original && delays==std::vector<unsigned>({20}),"Default resume restores palette, resets input and preserves continue count");
		reset();InputKeys shot;shot.shot=true;script={shot};require(!pause_menu() && !paused && !player_is_hit && continues==0,"Controller A mapped to Z alone resumes without simultaneous Enter exit");
        reset();script={{},escape};require(!pause_menu() && !paused && delays==std::vector<unsigned>({1,1,20}),"Escape release/repress resumes with original loop timing");
		reset();script={right_enter,{},enter};require(!pause_menu() && !paused && continues==0 && fb->palette==original,"Quit confirmation defaults to No and resumes");
		reset();script={right_enter,{},right_enter};require(pause_menu() && paused && continues==1 && fb->palette[7]==original[7] && fb->palette[10]==0xff444444u && delays==std::vector<unsigned>({20,1}),"Confirmed quit increments original continue counter and preserves dim-menu exit state");
		reset();InputKeys both=enter;both.shot=true;script={both};require(pause_menu() && player_is_hit && continues==0 && delays.empty(),"Original simultaneous shot/Enter early-exit quirk is preserved");
		if(argc>=2) {
			auto archive=load_archive(argv[1]);stg=read_sprites(asset(archive,"STG.PTN"));font=FontRom(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));auto scene=read_scene(asset(archive,"STAGE0.DAT"))[0];
			fb->palette=bg.palette;BombBackend real_bomb{*fb,kuji1,kuji2,[]() {}};BombBackendBinding real_bomb_binding(real_bomb,player);reset();
			for(unsigned page=0;page<2;++page) {fb->blit(page,0,0,bg.images[0],false);render_layout(*fb,scene,stg,page);}
			real=true;script={right_enter,{},right_enter};require(pause_menu() && continues==1,"Real font pause/confirmation sequence");
		}
		std::cout << "Original pause resume/Escape/confirmation, palette, counter and early-exit behavior passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
