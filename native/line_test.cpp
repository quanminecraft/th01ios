#include "line_backend.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
int main() {
    try {
        auto fb=std::make_unique<Framebuffer>();SpriteSet empty;ShotBackend shot{*fb,empty,{}};ShotBackendBinding binding(shot);
        fb->clear(0,0);fb->clear(1,5);page_access(0);graph_r_line(20,10,10,10,7);
        for(int x=0;x<32;++x) require(fb->pixel(0,x,10)==(x>=10 && x<=20 ? 7 : 0),"Horizontal special case is inclusive and orders endpoints");
        graph_r_line(50,20,50,10,2);for(int y=9;y<22;++y) require(fb->pixel(0,50,y)==(y>=10 && y<=20 ? 2 : 0),"Vertical special case is inclusive");
        fb->clear(0,0);graph_r_line(10,10,14,14,7);
        for(int i=0;i<4;++i) require(fb->pixel(0,10+i,10+i)==7,"Original sloped line flushes preceding cached pixels");
        require(fb->pixel(0,14,14)==0,"Original sloped line leaves its final byte unflushed");
        fb->clear(0,3);native_graph_line_unput(10,10,14,14);
        require(fb->pixel(0,0,10)==5 && fb->pixel(0,31,13)==5 && fb->pixel(0,0,15)==5,"Sloppy unput restores four-byte bands including advanced final row");
        require(fb->pixel(0,10,14)==3 && fb->pixel(0,32,10)==3,"Unput preserves original skipped endpoint row and bounded band width");
        fb->clear(0,0);fb->clear(1,5);page_access(1);graph_r_line(100,100,104,104,6);require(fb->pixel(1,100,100)==6 && fb->pixel(0,100,100)==0,"Line draw honors selected page");
        page_access(0);fb->clear(0,0);graph_r_line(-10,10,10,10,7);require(fb->pixel(0,0,10)==7 && fb->pixel(0,10,10)==7,"Horizontal clipping preserves visible endpoints");
        std::uint64_t trace=1469598103934665603ull;
        std::array<MimaPoint,8> ends{{{380,220},{340,260},{280,260},{240,220},{240,140},{280,100},{340,100},{380,140}}};
        for(auto end:ends) {
            fb->clear(0,3);graph_r_line(310,180,end.x,end.y,7);
            for(const auto& plane:fb->pages[0]) for(auto byte:plane) {trace^=byte;trace*=1099511628211ull;}
            native_graph_line_unput(310,180,end.x,end.y);
            for(const auto& plane:fb->pages[0]) for(auto byte:plane) {trace^=byte;trace*=1099511628211ull;}
        }
        std::cout<<"Shared original line loop, endpoint quirks, page selection and eight-octant restore trace "<<std::hex<<trace<<" passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}