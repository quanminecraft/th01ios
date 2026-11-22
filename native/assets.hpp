#pragma once
#include "archive.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>

namespace th01_native {
inline Bytes read_file(const std::filesystem::path& path) {
	std::ifstream in(path,std::ios::binary);
	if(!in) throw std::runtime_error("Cannot open asset file");
	return Bytes((std::istreambuf_iterator<char>(in)),{});
}
inline std::vector<Entry> load_archive(const std::filesystem::path& directory) {
	std::vector<Entry> result;
	unsigned matches=0;
	for(auto& f:std::filesystem::directory_iterator(directory)) {
		if(!f.is_regular_file()) continue;
		auto data=read_file(f.path());
		if(data.size()<2048 || data[0]!=0x95 || data[1]!=0x95 || data[2]!=3) continue;
		result=read_archive(data);
		++matches;
	}
	if(matches!=1) throw std::runtime_error("Expected exactly one TH01 archive");
	return result;
}
inline const Bytes& asset(const std::vector<Entry>& entries,const std::string& name) {
	for(auto& entry:entries) if(entry.name==name) return entry.data;
	throw std::runtime_error("Missing asset: "+name);
}
}
