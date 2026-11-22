#include "framebuffer.hpp"
#include "grf.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool condition) { if(!condition) throw std::runtime_error("Render test failed"); }
int main() {
	// PTN's 6-byte header, RGB palette, per-image marker, then B/R/G/E.
	Bytes ptn(54+513);
	std::copy_n("HPTN",4,ptn.begin());ptn[4]=1;ptn[5]=1;
	ptn[6+2*3]=15; // index 2 is red
	ptn[55]=0x80;ptn[55+128]=0x40;ptn[55+256]=0x20;ptn[55+384]=0x10;
	for(unsigned plane=0;plane<4;++plane) ptn[55+128*plane] |= 8;
	auto set=read_sprites(ptn);
	require(set.palette[2]==0xffff0000 && set.images.size()==1);
	auto& sprite=set.images[0];
	require(sprite.color[0]==1 && sprite.color[1]==2 && sprite.color[2]==4 && sprite.color[3]==8);
	require(sprite.color[4]==15 && !sprite.opaque[4] && sprite.opaque[0]);
	Bytes grc(65);std::copy_n("GRCG",4,grc.begin());grc[4]=1;grc[6]=1;grc[8]=1;grc[64]=0x81;
	auto mono=read_sprites(grc);
	require(mono.images[0].opaque[0] && mono.images[0].opaque[7] && !mono.images[0].opaque[1]);
	Bytes bos(69);std::copy_n("BOSS",4,bos.begin());bos[4]=1;bos[6]=1;bos[8]=1;
	bos[64]=0x7f;bos[65]=0x80;bos[66]=0x40;
	auto boss=read_sprites(bos);
	require(boss.images[0].color[0]==1 && boss.images[0].opaque[0] && !boss.images[0].opaque[1]);
	auto fb=std::make_unique<Framebuffer>();
	fb->palette=set.palette;
	fb->clear(0,7);fb->clear(1,2);
	fb->blit(0,15,0,sprite);
	require(fb->pixel(0,8,0)==1 && fb->pixel(0,9,0)==2 && fb->pixel(0,12,0)==7);
	fb->restore(1,0,8,0,2,1);
	require(fb->pixel(0,8,0)==2 && fb->pixel(0,10,0)==4 && fb->pixel(1,8,0)==2);
	fb->clear(0,7);fb->blit(0,-1,399,sprite,false);
	require(fb->pixel(0,0,399)==0 && fb->pixel(0,24,399)==7);
	fb->put(0,639,399,2);fb->put(0,640,400,1);
	require(fb->rgba(0).back()==0xffff0000);
	unsigned errors=0;
	try { ptn.pop_back();read_sprites(ptn); } catch(const std::runtime_error&) {++errors;}
	try {grc[4]=255;read_sprites(grc);} catch(const std::runtime_error&) {++errors;}
	try {fb->clear(0,16);} catch(const std::runtime_error&) {++errors;}
	require(errors==3);
	Bytes hud(64);std::copy_n("HGRF",4,hud.begin());hud[5]=0xfe;
	for(unsigned i=0;i<21;++i) {hud.push_back(0xfe);hud.push_back(255);hud.push_back(0x80);}
	hud[8]=63;
	auto mask=read_hud_mask(hud);
	require(mask.opaque[0] && !mask.opaque[1] && mask.opaque[65*640+632] && mask.color[0]==10);
	bool short_hud=false;
	try {hud.resize(64);read_hud_mask(hud);} catch(const std::runtime_error&) {short_hud=true;}
	require(short_hud);
	std::cout << "Sprite and framebuffer tests passed\n";
}
