#pragma once
#include "assets.hpp"
#include <memory>
namespace th01_native {
enum class SarielColumnKind {VerticalStacks,SealRain};
class SarielColumns {
 struct Impl;std::unique_ptr<Impl> impl;
public:
 SarielColumns(const std::vector<Entry>& archive,std::uint32_t seed,SarielColumnKind kind=SarielColumnKind::VerticalStacks);
 ~SarielColumns();
 SarielColumns(const SarielColumns&)=delete;
 SarielColumns& operator=(const SarielColumns&)=delete;
 bool advance();int frame()const;
};
}
