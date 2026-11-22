#pragma once
#include "framebuffer.hpp"
#include <fstream>
#include <filesystem>
namespace th01_native {
inline void dump_bmp(const std::filesystem::path& path,const Framebuffer& fb,unsigned page=0) {
	Bytes file(54+640*400*4);
	auto put=[&](unsigned offset,std::uint32_t value,unsigned count) {for(unsigned i=0;i<count;++i) file.at(offset+i)=value>>(8*i);};
	file[0]='B';file[1]='M';put(2,file.size(),4);put(10,54,4);put(14,40,4);
	put(18,640,4);put(22,static_cast<std::uint32_t>(-400),4);put(26,1,2);put(28,32,2);
	put(34,640*400*4,4);
	auto pixels=fb.rgba(page);
	for(unsigned i=0;i<pixels.size();++i) put(54+i*4,pixels[i],4);
	std::ofstream output(path,std::ios::binary);
	if(!output || !output.write(reinterpret_cast<const char*>(file.data()),file.size())) throw std::runtime_error("Cannot write screenshot");
}
}
