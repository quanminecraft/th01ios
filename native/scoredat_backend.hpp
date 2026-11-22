#pragma once
#include "physics_types.hpp"
#include "th01/score.h"
#include "th01/rank.h"
#include <filesystem>
#include <fstream>
#include <cstddef>
using shiftjis_kanji_t=std::uint16_t;
constexpr int SCOREDAT_PLACES=10,SCOREDAT_NAME_BYTES=16,SCOREDAT_NAMES_SIZE=160,SCOREDAT_NAME_KEY=0x9c;
constexpr int SCOREDAT_NAME_KANJI=8,SCOREDAT_CLEARED=40,SCOREDAT_CLEARED_MAKAI=50,SCOREDAT_CLEARED_JIGOKU=60;
#define SCOREDAT_MAGIC "HISCORE"
extern std::uint8_t rank;
extern std::int8_t* scoredat_names;
extern shiftjis_kanji_t* scoredat_routes;
extern std::int16_t* scoredat_stages;
extern score_t* scoredat_score;
int scoredat_load();void scoredat_save();void scoredat_free();
void scoredat_recreate();
score_t scoredat_hiscore_get();void scoredat_name_get(int place,unsigned char out[17]);
std::int8_t scoredat_name_byte_encode(std::int8_t value);std::int8_t scoredat_name_byte_decode(std::int8_t value);
namespace th01_native {
struct ScoredatBackend {std::filesystem::path directory;std::fstream file;};
class ScoredatBinding {
	ScoredatBackend* previous;
public:
	explicit ScoredatBinding(ScoredatBackend& backend);
	~ScoredatBinding();
	ScoredatBinding(const ScoredatBinding&)=delete;
	ScoredatBinding& operator=(const ScoredatBinding&)=delete;
};
}
bool file_exist(const char* name);int file_create(const char* name);int file_ropen(const char* name);
void file_write(const void* data,std::size_t size);void file_read(void* data,std::size_t size);void file_close();
