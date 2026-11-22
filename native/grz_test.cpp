#include "grz.hpp"
#include "assets.hpp"
#include "framebuffer_dump.hpp"
#include <iostream>
#include <memory>
using namespace th01_native;
static void require(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
static void put(Bytes& bytes,unsigned at,unsigned n,unsigned value) {for(unsigned i=0;i<n;++i) bytes.at(at+i)=value>>(i*8);}
static Bytes fixture() {
    // Put 1, skip 1 (zero run), put 3, skip 3, then skip remainder.
    Bytes commands{0,1,0,1,2,0,1,2,9};
    unsigned remaining=32000-8;
    while(remaining) {unsigned n=std::min(remaining,256u);if(n==1) commands.push_back(2);else {commands.push_back(1);commands.push_back(n-1);commands.push_back(2);}remaining-=n;}
    Bytes bytes(160+commands.size()+32);
    std::copy_n("HGRZ",4,bytes.begin());bytes[4]=1;put(bytes,8,4,96);put(bytes,72,4,bytes.size());
    std::copy_n("HGRX",4,bytes.begin()+96);bytes[100]=2;put(bytes,104,2,commands.size());put(bytes,106,2,16);
    std::copy(commands.begin(),commands.end(),bytes.begin()+160);
    auto data=160+commands.size();for(unsigned i=0;i<4;++i) {bytes[data+i*4]=0x80;bytes[data+16+i*4+1]=0x40;}
    return bytes;
}
int main(int argc,char** argv) {
    try {
        auto bytes=fixture();auto decoded=read_grz_image(bytes,0);
        require(decoded.images.size()==2,"Multiple planar streams share RLE");
        for(unsigned x=0;x<640*400;++x) {
            unsigned unit=x/8;bool opaque=unit==0 || (unit>=2 && unit<=4);
            require(bool(decoded.images[0].opaque[x])==opaque,"Run length and implicit extra unit");
            require(decoded.images[0].color[x]==(opaque && x%8==0 ? 1 : 0),"B plane order");
            require(decoded.images[1].color[x]==(opaque && x%8==1 ? 2 : 0),"R plane order and stream selection");
        }
        auto fb=std::make_unique<Framebuffer>();fb->clear(0,15);fb->blit(0,0,0,decoded.images[0],true);
        require(fb->pixel(0,0,0)==1 && fb->pixel(0,1,0)==0 && fb->pixel(0,8,0)==15,"Opaque zero replaces destination; skip preserves it");
        unsigned rejected=0;
        try {read_grz_image(bytes,1);}catch(const std::runtime_error&) {++rejected;}
        try {auto broken=bytes;broken.resize(159);read_grz_image(broken,0);}catch(const std::runtime_error&) {++rejected;}
        try {auto broken=bytes;put(broken,106,2,1);read_grz_image(broken,0);}catch(const std::runtime_error&) {++rejected;}
        require(rejected==3,"Bad image, truncation and planar underrun rejected");
        if(argc>=2) {
            auto real=read_file(std::filesystem::path(argv[1])/"BOSS8.GRZ");require(real.at(4)==16,"Original Konngara GRZ has sixteen images");
            for(unsigned i=0;i<real[4];++i) {
                auto image=read_grz_image(real,i);unsigned opaque=std::count(image.images[0].opaque.begin(),image.images[0].opaque.end(),1);
                require(opaque>0,"Konngara image has opaque pixels");
                fb->palette=image.palette;fb->clear(0,0);fb->blit(0,0,0,image.images[0],true);
                if(argc>=3) dump_bmp(std::string(argv[2])+"-"+std::to_string(i)+".bmp",*fb);
                std::cout<<"GRZ image "<<i<<" streams="<<image.images.size()<<" opaque="<<opaque<<'\n';
            }
        }
        std::cout<<"GRZ planar streams, run semantics and alpha passed\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
