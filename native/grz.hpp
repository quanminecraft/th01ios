#pragma once
#include "sprites.hpp"

namespace th01_native {
// Data translation of th01/formats/grz.cpp. Each opaque unit replaces all
// eight pixels, including zero bits; skipped units preserve the destination.
inline SpriteSet read_grz_image(const Bytes& file,unsigned image) {
    auto need=[&](std::size_t at,std::size_t n) {if(at>file.size() || n>file.size()-at) throw std::runtime_error("Truncated GRZ");};
    auto word=[&](std::size_t at) {need(at,2);return unsigned(file[at])|(unsigned(file[at+1])<<8);};
    auto dword=[&](std::size_t at) {need(at,4);return std::uint32_t(word(at))|(std::uint32_t(word(at+2))<<16);};
    need(0,96);
    if(std::string(file.begin(),file.begin()+4)!="HGRZ" || !file[4] || file[4]>16 || image>=file[4]) throw std::runtime_error("Invalid GRZ header/image");
    if(dword(72)!=file.size()) throw std::runtime_error("GRZ file size mismatch");
    auto base=std::size_t(dword(8+image*4));need(base,64);
    if(base<96 || std::string(file.begin()+base,file.begin()+base+4)!="HGRX" || !file[base+4] || file[base+4]>16) throw std::runtime_error("Invalid GRX header");
    unsigned count=file[base+4],rle_size=word(base+8),planar_size=word(base+10);
    std::size_t rle_start=base+64,planar_start=rle_start+rle_size;
    need(rle_start,rle_size);need(planar_start,std::size_t(count)*planar_size);
    SpriteSet result;
    for(unsigned c=0;c<16;++c) {
        auto at=base+16+c*3;unsigned r=file[at],g=file[at+1],b=file[at+2];
        if(r>15 || g>15 || b>15) throw std::runtime_error("Invalid GRX RGB4 palette");
        result.palette[c]=0xff000000u|(r*17u<<16)|(g*17u<<8)|b*17u;
    }
    // Expand the shared command stream once, preserving the original for-loop
    // increment after a run. A putting run therefore writes length+1 units;
    // a skipping run skips length+1. A zero-length run skips just one unit.
    std::vector<unsigned> opaque_units;std::size_t rle=0;
    auto command_byte=[&]() {if(rle>=rle_size) throw std::runtime_error("Truncated GRX commands");return file[rle_start+rle++];};
    for(unsigned offset=0;offset<32000;++offset) {
        unsigned command=command_byte();
        if(command==1) {
            unsigned runs=command_byte();
            if(runs) {
                command=command_byte();
                if(runs>31999-offset) throw std::runtime_error("GRX run outside image");
                while(runs--) {if(command==0) opaque_units.push_back(offset);++offset;}
            }
        }
        if(command==0) opaque_units.push_back(offset);
    }
    if(opaque_units.size()*4>planar_size) throw std::runtime_error("Truncated GRX planar stream");
    for(unsigned stream=0;stream<count;++stream) {
        Sprite sprite{640,400,Bytes(640*400),Bytes(640*400)};
        auto data=planar_start+std::size_t(stream)*planar_size;
        for(auto offset:opaque_units) {
            for(unsigned bit=0;bit<8;++bit) {
                auto pixel=offset*8+bit;sprite.opaque[pixel]=1;
                for(unsigned plane=0;plane<4;++plane) if(file[data+plane]&(0x80u>>bit)) sprite.color[pixel]|=1u<<plane;
            }
            data+=4;
        }
        result.images.push_back(std::move(sprite));
    }
    return result;
}
}
