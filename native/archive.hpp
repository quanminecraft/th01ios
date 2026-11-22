#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace th01_native {
using Bytes = std::vector<std::uint8_t>;
inline Bytes decode(const Bytes& packed, std::size_t size, bool compressed) {
	if(size > 16 * 1024 * 1024) throw std::runtime_error("Unreasonable asset size");
	std::size_t pos = 0;
	auto next = [&]() -> std::uint8_t {
		if(pos == packed.size()) throw std::runtime_error("Truncated asset");
		return packed[pos++] ^ 0x76;
	};
	Bytes out;
	out.reserve(size);
	if(!compressed) {
		if(size != packed.size()) throw std::runtime_error("Raw asset size mismatch");
		while(out.size() < size) out.push_back(next());
		return out;
	}
	if(!size) return out;
	auto value = next();
	unsigned runs = 0;
	while(out.size() < size) {
		out.push_back(value);
		// The DOS reader reads ahead beyond the final byte; the native reader
		// terminates here instead of relying on unrelated bytes after the asset.
		if(out.size() == size) break;
		if(runs) --runs;
		else {
			auto previous = value;
			value = next();
			if(previous == value) runs = next();
		}
	}
	return out;
}
struct Entry {
	std::string name;
	Bytes data;
};
inline std::vector<Entry> read_archive(const Bytes& file) {
	if(file.size() < 2048) throw std::runtime_error("Truncated archive directory");
	auto le = [&](std::size_t p, unsigned n) {
		std::uint32_t v = 0;
		for(unsigned i = 0; i < n; ++i) v |= std::uint32_t(file.at(p+i)) << (8*i);
		return v;
	};
	std::vector<Entry> entries;
	std::size_t directory_end = 0;
	while(directory_end < 2048 && file[directory_end]) directory_end += 32;
	if(directory_end < 2048) directory_end += 32;
	for(std::size_t p = 0; p < 2048 && file[p]; p += 32) {
		std::string name;
		for(unsigned i = 0; i < 13 && file[p+3+i]; ++i)
			name.push_back(static_cast<char>(file[p+3+i] ^ 0xff));
		if(name.empty() || name.size() == 13) throw std::runtime_error("Invalid asset name");
		for(auto& e : entries) if(e.name == name) throw std::runtime_error("Duplicate asset name");
		auto packed = le(p+16, 4), size = le(p+20, 4), offset = le(p+24, 4);
		if(offset < directory_end || offset > file.size() || packed > file.size()-offset)
			throw std::runtime_error("Invalid asset extent");
		Bytes payload(file.begin()+offset, file.begin()+offset+packed);
		entries.push_back({name, decode(payload, size, le(p,2) == 0x9595)});
	}
	return entries;
}
}
