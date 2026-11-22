#pragma once
#include "sariel_body.hpp"
#include "assets.hpp"
#include <memory>
namespace th01_native {
class SarielVortices {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    SarielVortices(SarielBody& body,const std::vector<Entry>& archive,std::uint32_t seed);
    ~SarielVortices();
    SarielVortices(const SarielVortices&)=delete;
    SarielVortices& operator=(const SarielVortices&)=delete;
    bool advance();
    int frame()const;
};
}
