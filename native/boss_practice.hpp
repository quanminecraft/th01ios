#pragma once
#include "game_options.hpp"
#include "th01/route.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <cstdio>
namespace th01_native {
enum class BossId : unsigned {shingyoku,yuugenmagan,mima,elis,kikuri,sariel,konngara};
enum class SessionPurpose {normal,boss_practice};
struct BossEntry {const char* name;unsigned stage;route_t route;};
inline const std::array<BossEntry,7>& boss_entries() {
 static const std::array<BossEntry,7> entries{{{"SHINGYOKU",5,ROUTE_MAKAI},{"YUUGENMAGAN",10,ROUTE_MAKAI},{"MIMA",10,ROUTE_JIGOKU},{"ELIS",15,ROUTE_MAKAI},{"KIKURI",15,ROUTE_JIGOKU},{"SARIEL",20,ROUTE_MAKAI},{"KONNGARA",20,ROUTE_JIGOKU}}};return entries;
}
inline std::optional<BossId> boss_for_stage(unsigned stage,route_t route) {
 for(unsigned i=0;i<7;++i)if(boss_entries()[i].stage==stage&&(stage==5||boss_entries()[i].route==route))return BossId(i);
 return {};
}
bool replace_progress_file(const std::filesystem::path& source,const std::filesystem::path& target);
class BossProgress {
 std::filesystem::path file;
 std::array<unsigned char,8> masks{};
 bool valid=true;
 std::string failure;
 static unsigned group(GameMode mode,unsigned rank) {
  if(rank>3||(mode!=GameMode::standard&&mode!=GameMode::challenge))throw std::invalid_argument("Invalid practice group");
  return (mode==GameMode::challenge?4:0)+rank;
 }
 static std::uint32_t checksum(const unsigned char* p,unsigned n) {std::uint32_t h=2166136261u;while(n--){h^=*p++;h*=16777619u;}return h;}
 bool save() {
  if(!valid)return false;
  std::error_code ec;std::filesystem::create_directories(file.parent_path(),ec);
  if(ec){failure="PROGRESS NOT SAVED";return false;}
  std::array<unsigned char,24> bytes{};const std::string magic="TH01OTHR";
  for(unsigned i=0;i<8;++i)bytes[i]=magic[i];bytes[8]=2;
  for(unsigned i=0;i<8;++i)bytes[12+i]=masks[i];
  auto sum=checksum(bytes.data(),20);for(unsigned i=0;i<4;++i)bytes[20+i]=sum>>(i*8);
  auto temp=file;temp+=".tmp";
  {std::ofstream out(temp,std::ios::binary|std::ios::trunc);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());out.flush();if(!out){failure="PROGRESS NOT SAVED";return false;}}
  if(!replace_progress_file(temp,file)){failure="PROGRESS NOT SAVED";std::filesystem::remove(temp,ec);return false;}
  failure.clear();return true;
 }
public:
 explicit BossProgress(const std::filesystem::path& profile):file(profile/"OTHER.DAT") {
  std::error_code ec;bool exists=std::filesystem::exists(file,ec);
  if(!exists&&!ec)return;
  if(ec||std::filesystem::file_size(file,ec)!=24||ec){valid=false;failure="OTHER.DAT ERROR";return;}
  std::ifstream in(file,std::ios::binary);std::string data((std::istreambuf_iterator<char>(in)),{});
  auto bad=[&](){valid=false;failure="OTHER.DAT ERROR";};
  if(ec||!in||data.size()!=24||data.substr(0,8)!="TH01OTHR"||(static_cast<unsigned char>(data[8])!=1&&static_cast<unsigned char>(data[8])!=2)||data[9]||data[10]||data[11]){bad();return;}
  std::uint32_t sum=0;for(unsigned i=0;i<4;++i)sum|=std::uint32_t(static_cast<unsigned char>(data[20+i]))<<(8*i);
  if(sum!=checksum(reinterpret_cast<const unsigned char*>(data.data()),20)){bad();return;}
  for(unsigned i=0;i<8;++i)if(data[8]==1&&(static_cast<unsigned char>(data[12+i])&128)){bad();return;}
  for(unsigned i=0;i<8;++i){masks[i]=static_cast<unsigned char>(data[12+i]);if(data[8]==1&&(masks[i]&0x60))masks[i]|=0x80;}
 }
 bool unlocked(GameMode mode,unsigned rank,BossId boss) const {
  auto bit=static_cast<unsigned>(boss);if(bit>=7)throw std::invalid_argument("Invalid boss");return (masks[group(mode,rank)]&(1u<<bit))!=0;
 }
 bool defeated(GameMode mode,unsigned rank,BossId boss) {
  auto bit=static_cast<unsigned>(boss);if(bit>=7)throw std::invalid_argument("Invalid boss");auto& mask=masks[group(mode,rank)];
  mask|=1u<<bit;if(boss==BossId::sariel||boss==BossId::konngara)mask|=0x80;return save();
 }
 bool completed(GameMode mode,unsigned rank)const{return (masks[group(mode,rank)]&0x80)!=0;}
 bool has_completion()const{for(auto mask:masks)if(mask&0x80)return true;return false;}
 const std::string& error() const {return failure;}
};
}
