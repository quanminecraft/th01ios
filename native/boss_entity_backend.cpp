#include "boss_entity_backend.hpp"
#include <cctype>
#include <cstdlib>
CBossEntity boss_entity_0{},boss_entity_1{},boss_entity_2{},boss_entity_3{},boss_entity_4{};
namespace {
th01_native::BossEntityBackend* active=nullptr;
th01_native::BossEntityBackend& backend() {if(!active) throw std::runtime_error("Boss entity backend is not bound");return *active;}
void draw(const CBossEntity& entity,screen_x_t left,screen_y_t top,int image,bool restore) {
	if(image<0 || image>=entity.bos_image_count) return;
	auto& b=backend();const auto& sprite=b.slots.at(entity.bos_slot).images.at(image);
	int row_offset=top*80+left/8;
	for(int y=0;y<entity.h;++y) {
		int intended=row_offset/80+(left<0 ? 1 : 0);
		for(int word=0;word<entity.vram_w/2;++word) {
			int offset=row_offset+word*2;
			if(offset<0 || offset+1>=32000 || offset/80!=intended || (restore && (offset+1)/80!=intended)) continue;
			for(int x=0;x<16;++x) {
				int index=y*sprite.width+word*16+x;int linear=offset*8+x;
				if(linear>=640*400) continue;
				unsigned page=restore ? 0 : th01_native::current_render_page();
				auto old=b.framebuffer.pixel(restore ? 1 : page,linear%640,linear/640);
				b.framebuffer.put(page,linear%640,linear/640,(sprite.opaque.at(index) ? 0 : old)|sprite.color.at(index));
			}
		}
		row_offset+=80;if(row_offset>=32000) break;
	}
	if(restore) page_access(0);
}
}
namespace th01_native {
BossEntityBinding::BossEntityBinding(BossEntityBackend& value):previous(active) {active=&value;}
BossEntityBinding::~BossEntityBinding() {active=previous;}
}
int CBossEntity::load(const char* name,int slot) {
	auto& b=backend();std::string key=name;for(auto& c:key) c=char(std::toupper(static_cast<unsigned char>(c)));
	auto images=th01_native::read_sprites(th01_native::asset(b.archive,key));
	if(images.images.size()>8 || images.images.at(0).width%16) throw std::runtime_error("Boss entity bank requires word-aligned width and at most eight cels");
	b.slots.at(slot)=std::move(images);bos_slot=slot;const auto& loaded=b.slots.at(slot);
	vram_w=loaded.images.at(0).width/8;h=loaded.images.at(0).height;bos_image_count=loaded.images.size();return 0;
}
void CBossEntity::put_8(screen_x_t left,vram_y_t top,int image) const {draw(*this,left,top,image,false);}
void CBossEntity::unput_and_put_8(screen_x_t left,vram_y_t top,int image) const {draw(*this,left,top,image,true);}
void bos_entity_free(int slot) {backend().slots.at(slot)={};}
#include "th01/main/boss/entity_motion.inc"

// Byte-shifted row renderer used by the original wave entrance routines.
// Clip per source word as entity_a.cpp does, rather than clipping the sprite
// rectangle; a shifted word may spill into the next framebuffer row.
void CBossEntity::put_1line(screen_x_t left,vram_y_t y,int image,pixel_t row) const {
    if(image<0 || image>=bos_image_count) return;
    auto& b=backend();const auto& sprite=b.slots.at(bos_slot).images.at(image);
    if(row<0 || row>=h) throw std::out_of_range("Boss sprite source row");
    int byte_left=left>=0 ? left/8 : -((-left+7)/8);
    int offset=y*80+byte_left;
    int intended=offset/80+(left<0 ? 1 : 0);
    unsigned shift=unsigned(left)&7;
    unsigned page=th01_native::current_render_page();
    for(int word=0;word<vram_w/2;++word) {
        int dst=offset+word*2;
        if(dst/80!=intended) continue;
        for(int x=0;x<16;++x) {
            int linear=dst*8+int(shift)+x;
            if(linear<0 || linear>=640*400) continue;
            auto i=std::size_t(row)*sprite.width+word*16+x;
            auto old=b.framebuffer.pixel(page,linear%640,linear/640);
            b.framebuffer.put(page,linear%640,linear/640,(sprite.opaque.at(i) ? 0 : old)|sprite.color.at(i));
        }
    }
}
void CBossEntity::unput_and_put_1line(screen_x_t left,vram_y_t y,int image,pixel_t row) const {
    if(image<0 || image>=bos_image_count) return;
    auto& b=backend();const auto& sprite=b.slots.at(bos_slot).images.at(image);
    if(row<0 || row>=h) throw std::out_of_range("Boss sprite source row");
    int offset=y*80+(left>=0 ? left/8 : -((-left+7)/8));
    int intended=offset/80+(left<0 ? 1 : 0);
    unsigned shift=unsigned(left)&7;
    for(int word=0;word<vram_w/2;++word) {
        int dst=offset+word*2;
        if(dst/80!=intended || dst<0 || dst+1>=th01_native::Framebuffer::plane_size) continue;
        std::uint16_t alpha=0;std::array<std::uint16_t,4> fg{};
        for(int x=0;x<16;++x) {
            auto i=std::size_t(row)*sprite.width+word*16+x;
            auto bit=std::uint16_t(1u<<((x<8 ? 7 : 23)-x));
            if(!sprite.opaque.at(i)) alpha|=bit;
            for(unsigned plane=0;plane<4;++plane) if(sprite.color.at(i)&(1u<<plane)) fg[plane]|=bit;
        }
        auto mask=shift ? std::uint16_t((unsigned(alpha)>>shift)+(unsigned(alpha)<<(16-shift))) : alpha;
        for(unsigned plane=0;plane<4;++plane) {
            auto& bg=b.framebuffer.pages[1][plane];auto& out=b.framebuffer.pages[0][plane];
            unsigned background=(unsigned(bg[dst])|(unsigned(bg[dst+1])<<8))&mask;
            // Source uses signed sdots16_t and arithmetic right shift here.
            // Multiplication avoids undefined left shift of a negative value.
            std::int32_t value=fg[plane]<0x8000 ? fg[plane] : int(fg[plane])-0x10000;
            if(shift) {
                int divisor=1<<shift;
                int right=value>=0 ? value/divisor : -((-value+divisor-1)/divisor);
                value=right+value*(1<<(16-shift));
            }
            auto result=std::uint16_t(value)|background;
            out[dst]=std::uint8_t(result);out[dst+1]=std::uint8_t(result>>8);
        }
    }
    page_access(0);
}
#include "th01/math/wave.hpp"
#include "th01/main/boss/entity_wave.inc"

// CBossAnim owns separate BOS banks from collidable boss entities. Decoded
// opacity already represents the original load-time inverted alpha plane.
CBossAnim boss_anims[2]{};
int CBossAnim::load(const char* name,int slot){
    auto& b=backend();std::string key=name;for(auto& c:key)c=char(std::toupper(static_cast<unsigned char>(c)));
    auto sprites=th01_native::read_sprites(th01_native::asset(b.archive,key));
    if(sprites.images.empty() || sprites.images.size()>8 || sprites.images[0].width%16)throw std::runtime_error("Boss animation requires word-aligned BOS bank");
    b.animation_slots.at(slot)=std::move(sprites);auto& loaded=b.animation_slots.at(slot);bos_slot=slot;vram_w=loaded.images[0].width/8;h=loaded.images[0].height;bos_image_count=loaded.images.size();return 0;
}
void CBossAnim::put_8()const{
    if(bos_image>=bos_image_count)return;auto& b=backend();auto& sprite=b.animation_slots.at(bos_slot).images.at(bos_image);auto page=th01_native::current_render_page();int row=top*80+left/8;
    for(int y=0;y<h;++y){int intended=row/80+(left<0 ? 1:0);
        for(int word=0;word<vram_w/2;++word){int off=row+word*2;if(off<0 || off+1>=32000 || off/80!=intended)continue;
            for(int x=0;x<16;++x){int linear=off*8+x,index=y*sprite.width+word*16+x;if(linear>=640*400)continue;auto old=b.framebuffer.pixel(page,linear%640,linear/640);b.framebuffer.put(page,linear%640,linear/640,(sprite.opaque.at(index) ? 0:old)|sprite.color.at(index));}
        }
        row+=80;if(row>=32000)break;
    }
}
void bos_anim_free(int slot){backend().animation_slots.at(slot)={};}