#include <filesystem>
#include <cstdio>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
namespace th01_native {
bool replace_progress_file(const std::filesystem::path& source,const std::filesystem::path& target) {
#ifdef _WIN32
 return MoveFileExW(source.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
 return std::rename(source.c_str(),target.c_str())==0;
#endif
}
}
