#pragma once
#include "framebuffer.hpp"

namespace th01_native {
struct StageObject {
	int left,top;
	std::uint8_t type;
	std::uint8_t hp; // cards: remaining extra flips, as in CCards::hp
};
struct StageLayout {
	std::vector<StageObject> cards,obstacles;
};
using Scene=std::array<StageLayout,5>;
inline Scene read_scene(const Bytes& file) {
	if(file.size()!=1272 || std::string(file.begin(),file.begin()+5)!="STAGE")
		throw std::runtime_error("Invalid STAGE data");
	Scene scene;
	for(unsigned stage=0;stage<5;++stage) {
		auto base=22+stage*250;
		for(unsigned y=0;y<10;++y) for(unsigned x=0;x<20;++x) {
			auto type=file[base+50+y*20+x];
			bool card=(file[base+y*5+x/4]&(8u>>(x%4)))!=0;
			int left=x*32,top=64+y*32;
			if(card) scene[stage].cards.push_back({left,top,0,std::uint8_t(type>=14 && type<=16 ? type-13 : 0)});
			if((type>=1 && type<=13) || (type>=17 && type<=21))
				scene[stage].obstacles.push_back({left,top,type,0});
			// Other values are ignored by upstream stageobjs_init_and_render.
		}
	}
	return scene;
}
inline void render_layout(Framebuffer& fb,const StageLayout& stage,const SpriteSet& sprites,unsigned page=0) {
	// Sprite IDs from th01/sprites/main_ptn.h and stageobj.cpp CARD_ANIM.
	static constexpr unsigned card_cels[]={17,14,11,8};
	for(auto& object:stage.obstacles) {
		unsigned cel=object.type==1 ? 22 : object.type<=13 ? 23 : object.type==17 ? 29
			: object.type==18 ? 25 : object.type==19 ? 27 : object.type==20 ? 26 : 28;
		fb.blit(page,object.left,object.top,sprites.images.at(cel));
	}
	for(auto& card:stage.cards) fb.blit(page,card.left,card.top,sprites.images.at(card_cels[card.hp]));
}
}
