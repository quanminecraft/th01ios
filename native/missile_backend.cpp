#include "missile_backend.hpp"
namespace {
th01_native::MissileBackend* active=nullptr;
th01_native::MissileBackend& backend() {if(!active) throw std::runtime_error("Missile backend is not bound");return *active;}
}
namespace th01_native {
MissileBinding::MissileBinding(MissileBackend& value):previous(active) {active=&value;}
MissileBinding::~MissileBinding() {active=previous;}
}
std::int16_t native_missile_frame() {return backend().frame;}
void native_missile_load(main_ptn_slot_t slot) {
    auto& b=backend();b.sprites=th01_native::read_sprites(th01_native::asset(b.archive,"BOSS3_M.PTN"));b.slot=slot;
}
void native_missile_put(screen_x_t left,screen_y_t top,int id,int quarter) {
    auto& b=backend();int image=id-PTN_ID(b.slot,0);
    const auto& sprite=b.sprites.images.at(image);
    if(quarter<0 || quarter>3 || sprite.width!=32 || sprite.height!=32) throw std::runtime_error("Invalid missile sprite quarter");
    auto& fb=th01_native::graphics_framebuffer();auto page=th01_native::current_render_page();
    unsigned shift=unsigned(left)&7;
    int base=top*80+left/8;
    for(int y=0;y<16;++y) {
        std::uint16_t mask=0;std::array<std::uint16_t,4> planes{};
        for(int x=0;x<16;++x) {
            auto i=((quarter>>1)*16+y)*32+(quarter&1)*16+x;
            unsigned bit=1u<<((x<8 ? 7 : 23)-x);
            if(sprite.opaque.at(i)) mask|=bit;
            for(unsigned p=0;p<4;++p) if(sprite.color.at(i)&(1u<<p)) planes[p]|=bit;
        }
        auto shifted=[&](std::uint16_t value) {
            std::array<std::uint8_t,3> out{};
            if(!shift) {out[0]=std::uint8_t(value);out[1]=std::uint8_t(value>>8);}
            else {
                out[0]=std::uint8_t((value&0xff)>>shift);
                out[1]=std::uint8_t(((value&0xff)<<(8-shift))|((value&0xff00)>>(8+shift)));
                // Truncation to byte retains the low tail shifted out of byte two.
                out[2]=std::uint8_t((value&0xff00)>>shift);
            }
            return out;
        };
        auto alpha=shifted(mask);
        for(unsigned p=0;p<4;++p) {
            auto color=shifted(planes[p]);auto& out=fb.pages[page][p];
            for(int byte=0;byte<3;++byte) {int dst=base+y*80+byte;if(dst<0 || dst>=th01_native::Framebuffer::plane_size) continue;out[dst]=(out[dst]&~alpha[byte])|(color[byte]&alpha[byte]);}
        }
    }
}