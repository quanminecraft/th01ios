#include "boss_entity_backend.hpp"
#include "grp.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
int main(int argc,char** argv) {
	try {
		if(argc<2 || argc>3) throw std::invalid_argument("Usage: boss_entity_test ASSET_DIR [BMP]");
		auto archive=load_archive(argv[1]);auto fb=std::make_unique<Framebuffer>();auto stg=read_sprites(asset(archive,"STG.PTN"));
		ShotBackend shots{*fb,stg,{}};ShotBackendBinding shot_binding(shots);BossEntityBackend backend{*fb,archive,{}};BossEntityBinding binding(backend);
		CBossEntity entity{};entity.load("boss1.bos",0);require(entity.h==96 && entity.vram_w==12 && entity.bos_image_count==8,"Original Singyoku sphere metadata");
		entity.pos_set(100,100,0,200,64,200);entity.set_image(0);entity.lock_frame=0;entity.locked_move_and_put_8(300,300,3);
		require(entity.cur_left==200 && entity.cur_top==200 && entity.prev_left==100 && entity.prev_top==100 && entity.lock_frame==1,"Shared original movement clamps and records previous coordinates");
		entity.locked_move_and_put_8(-8,-8,3);entity.locked_move_and_put_8(-8,-8,3);entity.locked_move_and_put_8(-8,-8,3);
		require(entity.cur_left==200 && entity.lock_frame==0,"Original lock interval postpones movement until following call");
		entity.hitbox_orb_set(8,8,88,88);entity.hitbox_orb_activate();orb_prev_left=232;orb_prev_top=232;orb_cur_left=0;orb_cur_top=0;
		require(entity.hittest_orb(),"Entity collision uses previous orb coordinates");entity.hitbox_orb_deactivate();require(!entity.hittest_orb(),"Inactive orb hitbox suppresses collision");
		fb->clear(0,3);fb->clear(1,5);page_access(0);entity.unput_and_put_8(104,100,0);
		const auto& sprite=backend.slots[0].images[0];unsigned opaque=0,transparent=0;
		for(int y=0;y<96;++y) for(int x=0;x<96;++x) {unsigned i=y*96+x;require(fb->pixel(0,104+x,100+y)==((sprite.opaque[i] ? 0 : 5)|sprite.color[i]),"BOS unput/put preserves raw foreground planes and page-1 alpha background");if(sprite.opaque[i]) ++opaque;else ++transparent;}
		require(opaque>0 && transparent>0,"Real sphere covers both opaque and transparent pixels");
		for(int shift=0;shift<8;++shift) {
            fb->clear(0,3);page_access(0);entity.put_1line(104+shift,100,0,17);
            for(int y=99;y<=101;++y) for(int x=96;x<216;++x) {
                int sx=x-104-shift;unsigned expected=3;
                if(y==100 && sx>=0 && sx<96) {auto i=17*96+sx;expected=(sprite.opaque[i] ? 0 : 3)|sprite.color[i];}
                require(fb->pixel(0,x,y)==expected,"Row renderer shifts all eight pixel offsets without touching adjacent rows");
            }
        }
        fb->clear(0,3);entity.put_1line(639,100,0,17);
        for(int x=0;x<16;++x) {int linear=100*640+639+x;auto i=17*96+x;require(fb->pixel(0,linear%640,linear/640)==((sprite.opaque[i] ? 0 : 3)|sprite.color[i]),"Original right-edge source word spills into following row");}
        fb->clear(0,3);entity.put_1line(-8,100,0,17);
        require(fb->pixel(0,0,100)==3,"Original negative-left clipping skips first whole word");
        for(int x=0;x<16;++x) {auto i=17*96+16+x;require(fb->pixel(0,8+x,100)==((sprite.opaque[i] ? 0 : 3)|sprite.color[i]),"Negative-left clipping resumes with second source word");}
        auto unchanged=fb->pages;entity.put_1line(104,100,8,17);require(fb->pages==unchanged,"Row renderer ignores unavailable cels");
        auto before=fb->pages;entity.put_8(100,100,8);require(fb->pages==before,"Out-of-range cel is ignored as in source");
		// Aligned row restore must match the existing full-sprite renderer.
        fb->clear(0,3);fb->clear(1,5);page_access(1);entity.unput_and_put_1line(104,100,0,17);
        require(current_render_page()==0,"Row restore always returns to page zero");
        for(int x=0;x<96;++x) {auto i=17*96+x;require(fb->pixel(0,104+x,100)==((sprite.opaque[i] ? 0 : 5)|sprite.color[i]),"Aligned row restores page-one alpha background");}
        auto& fixture=backend.slots[0].images[0];fixture.color.assign(96*96,0);fixture.opaque.assign(96*96,0);
        fixture.color[8]=1;fixture.opaque[8]=1;
        fb->clear(0,3);fb->clear(1,5);entity.unput_and_put_1line(105,100,0,0);
        int at=100*80+13;
        require(fb->pages[0][0][at]==0xff && fb->pages[0][0][at+1]==0xff,"Signed foreground shift preserves original sign extension");
        require(fb->pages[0][2][at]==0xff && fb->pages[0][2][at+1]==0xbf,"Unsigned alpha rotates separately from signed foreground");
        require(fb->pages[0][1][at]==0 && fb->pages[0][3][at+1]==0,"Restored row replaces old foreground planes");
        auto restored=fb->pages;entity.unput_and_put_1line(105,100,8,0);require(fb->pages==restored,"Invalid restore cel leaves both pages unchanged");
        entity.load("boss1_2.bos",1);CBossEntity person{};person.load("boss1_3.bos",2);
		auto bg=read_grp(read_file(std::filesystem::path(argv[1])/"BOSS1.GRP"));fb->palette=bg.palette;for(unsigned page=0;page<2;++page) fb->blit(page,0,0,bg.images[0],false);
		CBossEntity sphere{};sphere.load("boss1.bos",0);sphere.put_8(80,120,0);person.put_8(272,120,0);person.put_8(464,120,3);
		if(argc==3) dump_bmp(argv[2],*fb);
		bos_entity_free(0);require(backend.slots[0].images.empty(),"Entity sprite banks release independently");
		std::cout<<"Real Singyoku entities, shared movement/lock/orb collision and alpha rendering passed\n";
	} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
