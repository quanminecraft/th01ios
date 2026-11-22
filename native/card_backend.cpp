#include "card_backend.hpp"

th01_native::CardState cards;
std::vector<std::uint32_t> cards_score;
std::int32_t score=0;
bool16 test_damage=false;
bool16 stage_cleared=false;
std::int8_t stage_num=1;
#include "th01/main/stage/card_anim.inc"
namespace {
th01_native::CardEvents* active=nullptr;
th01_native::CardEvents& events() {
	if(!active) throw std::runtime_error("Card event backend is not bound");
	return *active;
}
th01_native::CardRenderer& renderer() {
	if(!events().renderer) throw std::runtime_error("Card renderer is not bound");
	return *events().renderer;
}
}
namespace th01_native {
CardBackendBinding::CardBackendBinding(CardEvents& value):previous(active) {active=&value;}
CardBackendBinding::~CardBackendBinding() {active=previous;}
void load_cards(const StageLayout& layout) {
	if(layout.cards.size()>200) throw std::runtime_error("Too many cards for stage grid");
	cards={};cards.count=static_cast<std::int16_t>(layout.cards.size());
	for(auto& card:layout.cards) {
		if(card.hp>=CARD_HP_MAX) throw std::runtime_error("Invalid card HP");
		cards.left.push_back(card.left);cards.top.push_back(card.top);cards.flag.push_back(CARD_ALIVE);cards.hp.push_back(card.hp);
	}
	cards.flip_frame.assign(layout.cards.size(),0);
	cards_score.assign(layout.cards.size(),0);
}
void capture_card_backgrounds(CardRenderer& render) {
	render.backgrounds.clear();
	for(int i=0;i<cards.count;++i) {
		Sprite bg{32,32,Bytes(1024),Bytes(1024,1)};
		for(int y=0;y<32;++y) for(int x=0;x<32;++x)
			bg.color[y*32+x]=render.framebuffer.pixel(1,cards.left[i]+x,cards.top[i]+y);
		render.backgrounds.push_back(std::move(bg));
	}
}
}
void hud_score_and_cardcombo_render() {
	events().hud.push_back({score,cardcombo_cur,cardcombo_max});
	if(events().hud_dispatch) events().hud_dispatch();
}
void items_point_add(unsigned card) {
	events().items.push_back({card,false});
	if(events().item_dispatch) events().item_dispatch(card,false);
}
void items_bomb_add(unsigned card) {
	events().items.push_back({card,true});
	if(events().item_dispatch) events().item_dispatch(card,true);
}
void native_emit_pellet_group(screen_x_t left,screen_y_t top,pellet_group_t group,subpixel_t speed) {
	events().pellets.push_back({left,top,group,speed});
	if(events().pellet_dispatch) events().pellet_dispatch(left,top,group,speed);
}
void stageobj_put_bg_and_obj_8(screen_x_t left,screen_y_t top,int id,int slot) {
	auto& render=renderer();
	render.framebuffer.blit(th01_native::current_render_page(),left,top,render.backgrounds.at(slot),false);
	render.framebuffer.blit(th01_native::current_render_page(),left,top,render.sprites.images.at(id));
}
void graph_putsa_fx(screen_x_t left,screen_y_t top,int16_t effect,const char* text) {
	auto& render=renderer();render.font.shift_jis(render.framebuffer,th01_native::current_render_page(),left,top,effect,text);
}
