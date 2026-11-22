#include "hud_layout.hpp"
#include "assets.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <memory>
#include <iostream>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc>3) throw std::invalid_argument("Usage: hud_layout_test [ASSET_DIR [SCREENSHOT.bmp]]");
		Bytes packed={0xaa,2,0x80,0xaa,0};unsigned remaining=5280-4;
		while(remaining) {unsigned amount=std::min(remaining,256u);if(amount==1) packed.push_back(0);else {packed.push_back(0xaa);packed.push_back(amount-1);packed.push_back(0);}remaining-=amount;}
		Bytes grf(64);grf[0]='H';grf[1]='G';grf[2]='R';grf[3]='F';grf[5]=0xaa;grf[8]=packed.size();grf[9]=packed.size()>>8;grf.insert(grf.end(),packed.begin(),packed.end());
		auto mask=read_hud_mask(grf);require(mask.dots.size()==5280 && mask.dots[0]==0x80 && mask.dots[2]==0x80 && mask.dots[3]==0xaa,"Original RLE repeat-plus-one and escaped marker");
		bool failed=false;auto bad=grf;bad.resize(65);try {read_hud_mask(bad);} catch(const std::runtime_error&) {failed=true;}require(failed,"Truncated mask rejected");
		auto fb=std::make_unique<Framebuffer>();SpriteSet sprites;
		for(int i=0;i<34;++i) sprites.images.push_back({32,32,Bytes(1024,7),Bytes(1024,1)});
		Bytes font_bytes(0x46800);for(unsigned c=0x21;c<=0x7e;++c) font_bytes[0x1800+(0x29-0x21)*96*32+(c-0x20)*32]=0x80;
		for(unsigned d=0;d<10;++d) {font_bytes[0x1800+(0x29-0x21)*96*32+('0'+d-0x20)*32]=0x80u>>(d%8);font_bytes[0x1800+(0x23-0x21)*96*32+(0x30+d-0x20)*32]=0x80u>>(d%8);}
		FontRom font(std::move(font_bytes));ShotBackend shots{*fb,sprites,{}};ShotBackendBinding shot_binding(shots);
		CardRenderer renderer{*fb,sprites,font,{}};CardEvents events;events.renderer=&renderer;CardBackendBinding card_binding(events);HudScoreBinding score_binding(events);
		std::int16_t lives=4;HudIconsBackend hud{*fb,lives,{}};hud.hiscore=5000;HudIconsBinding hud_binding(hud);
		TimerBackend timer{*fb};TimerBackendBinding timer_binding(timer);HudLayoutBackend layout{*fb,mask};HudLayoutBinding layout_binding(layout);
		fb->clear(0,3);fb->clear(1,5);score=0;cardcombo_cur=0;cardcombo_max=0;rem_bombs=3;rank=RANK_NORMAL;stage_num=1;first_stage_in_scene=true;timer_init_for(0,ROUTE_MAKAI);native_hud_layout_render();
		require(fb->pixel(0,0,0)==2 && fb->pixel(1,0,0)==2 && fb->pixel(0,128,0)==7 && fb->pixel(1,128,0)==7,"Full HUD draws mask and icons on both pages");
		require(fb->pixel(0,616,32)==7 && fb->pixel(1,616,32)==7 && fb->pixel(0,576,48)==7,"Stage and centered normal rank positions");
		require(!fwnum_force_rerender && current_render_page()==0 && stage_timer==1000,"HUD startup preserves timer and finishes forced redraw on page zero");
		first_stage_in_scene=false;stage_num=2;native_hud_layout_render();
		require(fb->pixel(0,616,32)==3 && fb->pixel(1,616,32)==3 && fb->pixel(0,618,32)==7,"Later stage restores previous stage number background");
		if(argc>=2) {
			auto archive=load_archive(argv[1]);auto real_sprites=read_sprites(asset(archive,"STG.PTN"));auto scene=read_scene(asset(archive,"STAGE0.DAT"))[0];auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"ST0.GRP"));
			FontRom real_font(read_file(std::filesystem::path(argv[1])/"FONT.ROM"));auto real_mask=read_hud_mask(read_file(std::filesystem::path(argv[1])/"MASK.GRF"));
			require(real_mask.dots.size()==5376 && real_mask.consumed==2365,"Real mask retains original final run overshoot and consumed byte count");
			ShotBackend real_shots{*fb,real_sprites,{}};ShotBackendBinding real_shot_binding(real_shots);CardRenderer real_renderer{*fb,real_sprites,real_font,{}};events.renderer=&real_renderer;
			HudIconsBackend real_hud{*fb,lives,{}};real_hud.hiscore=100000;HudIconsBinding real_hud_binding(real_hud);TimerBackend real_timer{*fb};TimerBackendBinding real_timer_binding(real_timer);HudLayoutBackend real_layout{*fb,real_mask};HudLayoutBinding real_layout_binding(real_layout);
			fb->palette=bg.palette;for(unsigned page=0;page<2;++page) {fb->blit(page,0,0,bg.images[0],false);render_layout(*fb,scene,real_sprites,page);}
			score=12345;cardcombo_cur=17;stage_num=1;first_stage_in_scene=true;timer_init_for(0,ROUTE_MAKAI);native_hud_layout_render();
			require(real_hud.hiscore==100000 && hud_cardcombo_max==17,"Real complete HUD setup preserves hi-score and draws combo");
			if(argc==3) dump_bmp(argv[2],*fb);
		}
		std::cout << "Original MASK.GRF RLE, full HUD setup, stage/rank and later-stage redraw passed\n";
	} catch(const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}
