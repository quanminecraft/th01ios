#include "archive.hpp"
#include "sprites.hpp"
#include "grp.hpp"
#include "stage.hpp"
#include "grf.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

static std::uint64_t digest(const th01_native::Bytes& bytes) {
	std::uint64_t hash=14695981039346656037ull;
	for(auto value:bytes) {hash^=value;hash*=1099511628211ull;}
	return hash;
}
static std::uint64_t raster_digest(const th01_native::SpriteSet& set) {
	th01_native::Bytes bytes;
	for(auto rgba:set.palette) for(unsigned shift=0;shift<32;shift+=8) bytes.push_back(rgba>>shift);
	for(auto& sprite:set.images) {
		bytes.insert(bytes.end(),sprite.color.begin(),sprite.color.end());
		bytes.insert(bytes.end(),sprite.opaque.begin(),sprite.opaque.end());
	}
	return digest(bytes);
}
int main(int argc, char** argv) {
	try {
		if(argc != 2) throw std::runtime_error("Usage: asset_check <extracted game directory>");
		std::size_t archives = 0, count = 0, total = 0;
		std::size_t grp_count=0,sprite_count=0;
		for(auto& f : std::filesystem::directory_iterator(argv[1])) {
			// Identify the Japanese-named archive by its wire header without
			// converting the original CP932 filename or assuming an extension.
			if(!f.is_regular_file()) continue;
			std::ifstream in(f.path(), std::ios::binary);
			th01_native::Bytes data((std::istreambuf_iterator<char>(in)), {});
			if(f.path().filename()=="MASK.GRF") th01_native::read_hud_mask(data);
			if(f.path().extension()==".GRP") {
				auto image=th01_native::read_grp(data);
				std::cout << f.path().filename().string() << " raster " << std::hex << raster_digest(image) << std::dec << '\n';
				++grp_count;
				if(image.images[0].width!=640 || image.images[0].height!=400) throw std::runtime_error("Unexpected TH01 GRP dimensions");
			}
			if(data.size() < 2048 || data[0] != 0x95 || data[1] != 0x95 || data[2] != 3) continue;
			auto entries = th01_native::read_archive(data);
			++archives;
			for(auto& e : entries) {
				std::cout << e.name << '\t' << e.data.size() << " digest " << std::hex << digest(e.data) << std::dec << '\n';
				if(e.name.substr(0,5)=="STAGE") {
					auto scene=th01_native::read_scene(e.data);
					for(unsigned i=0;i<4;++i) std::cout << "  layout " << i << ": " << scene[i].cards.size() << " cards, " << scene[i].obstacles.size() << " obstacles\n";
				}
				if(e.name.size()>4 && (e.name.substr(e.name.size()-4)==".PTN" || e.name.substr(e.name.size()-4)==".GRC" || e.name.substr(e.name.size()-4)==".BOS")) {
					auto sprites=th01_native::read_sprites(e.data);
					sprite_count+=sprites.images.size();
					std::cout << "  " << sprites.images.size() << " sprites " << sprites.images[0].width << "x" << sprites.images[0].height << '\n';
					std::cout << "  raster " << std::hex << raster_digest(sprites) << std::dec << '\n';
				}
				++count;
				total += e.data.size();
			}
		}
		if(!archives) throw std::runtime_error("No TH01 archive found");
		std::cout << "Validated " << archives << " archive(s), " << count << " assets, " << total << " decoded bytes\n";
		std::cout << "Decoded " << sprite_count << " sprites and " << grp_count << " GRP images\n";
	} catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
