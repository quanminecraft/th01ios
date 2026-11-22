#include "text_animation.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc>3) throw std::invalid_argument("Usage: text_animation_test [ASSET_DIR [BMP_PREFIX]]");
		auto fb=std::make_unique<Framebuffer>();fb->clear(0,3);fb->clear(1,5);fb->palette.fill(0xff102030u);
		auto before=std::make_unique<decltype(fb->pages)>(fb->pages);
		Bytes bytes(0x46800);bytes[0x800+'H'*16]=0x80;bytes[0x800+'U'*16]=0x40;bytes[0x800+'P'*16+15]=1;
		FontRom font(std::move(bytes));unsigned phases=0;
		auto wait=[&](unsigned frames) {
			require(frames==27,"Original phrase wait is 27 frames");++phases;
			auto pixels=fb->rgba(0);unsigned red=0;for(auto color:pixels) red+=color==0xffff0000u;
			if(phases==1) require(red==256 && pixels[80*640]==0xffff0000u && pixels[80*640+16]==0xff102030u,"HARRY starts at row five with 16x dot scale");
			else require(red==512 && pixels[80*640+208]==0xffff0000u && pixels[320*640+432]==0xffff0000u && pixels[80*640]==0xff102030u,"UP is centered and replaces first phrase");
			require(fb->rgba(1)==pixels,"Text overlay appears over either graphics page");
		};
		SpriteSet sprites;ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		TimerBackend timer{*fb};TimerBackendBinding timer_binding(timer);
		{HarryUpAnimationBinding binding(timer,font,wait);native_harryup_animate();}
		require(phases==2 && fb->text_overlay.empty() && fb->pages==*before && shots.sound_commands==std::vector<unsigned>({17}),"Animation clears text, preserves both graphics pages and emits SE once");
		bool failed=false;try {harryup_text_animate(*fb,font,[](unsigned) {throw std::runtime_error("Host interrupted animation");});} catch(const std::runtime_error&) {failed=true;}
		require(failed && fb->text_overlay.empty() && fb->pages==*before,"Interrupted animation clears overlay without changing graphics");
		if(argc>=2) {
			auto archive=load_archive(argv[1]);auto stg=read_sprites(asset(archive,"STG.PTN"));auto layout=read_scene(asset(archive,"STAGE0.DAT"))[0];
			auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));FontRom real_font(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));
			fb->palette=bg.palette;fb->blit(0,0,0,bg.images[0],false);render_layout(*fb,layout,stg,0);unsigned phase=0;
			harryup_text_animate(*fb,real_font,[&](unsigned n) {
				require(n==27,"Real animation timing");++phase;
				require(std::count(fb->text_overlay.begin(),fb->text_overlay.end(),0xffff0000u)>0,"Real ANK glyphs produce enlarged text");
				if(argc==3) dump_bmp(std::string(argv[2])+"-"+std::to_string(phase)+".bmp",*fb);
			});
			require(phase==2 && fb->text_overlay.empty(),"Real phrase sequence and cleanup");
		}
		std::cout << "Native original ANK HARRY/UP geometry, 27+27 timing, overlay/SE and cleanup passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
