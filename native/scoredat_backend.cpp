#include "scoredat_backend.hpp"
#include <stdexcept>
namespace {
th01_native::ScoredatBackend* active=nullptr;
th01_native::ScoredatBackend& backend() {if(!active) throw std::runtime_error("Native score directory is not bound");return *active;}
std::filesystem::path path_for(const char* name) {
	std::filesystem::path name_path(name);if(name_path.empty() || name_path!=name_path.filename() || name_path=="." || name_path=="..") throw std::invalid_argument("Score filename must be a leaf name");
	return backend().directory/name_path;
}
int open_file(const char* name,std::ios::openmode mode) {auto& file=backend().file;if(file.is_open()) file.close();file.clear();file.open(path_for(name),mode|std::ios::binary);return bool(file);}
}
namespace th01_native {
ScoredatBinding::ScoredatBinding(ScoredatBackend& value):previous(active) {
	std::uint16_t probe=1;if(*reinterpret_cast<unsigned char*>(&probe)!=1) throw std::runtime_error("Original score I/O requires a little-endian host");
	std::filesystem::create_directories(value.directory);active=&value;
}
ScoredatBinding::~ScoredatBinding() {if(active && active->file.is_open()) active->file.close();active=previous;}
}
bool file_exist(const char* name) {return std::filesystem::is_regular_file(path_for(name));}
int file_create(const char* name) {return open_file(name,std::ios::out|std::ios::trunc);}
int file_ropen(const char* name) {if(!file_exist(name) || std::filesystem::file_size(path_for(name))<247) return 0;return open_file(name,std::ios::in);}
void file_write(const void* data,std::size_t size) {if(!backend().file.write(static_cast<const char*>(data),size)) throw std::runtime_error("Cannot write original score data");}
void file_read(void* data,std::size_t size) {if(!backend().file.read(static_cast<char*>(data),size)) throw std::runtime_error("Truncated original score data");}
void file_close() {backend().file.close();}
