#pragma once
#include "physics_types.hpp"
#include <functional>
#include <memory>
namespace th01_native {
class MimaPillarPattern {
    struct Impl;
    std::unique_ptr<Impl> impl;
public:
    explicit MimaPillarPattern(std::function<void(bool)> casting,screen_x_t left=256,screen_y_t top=120);
    ~MimaPillarPattern();
    MimaPillarPattern(const MimaPillarPattern&)=delete;
    MimaPillarPattern& operator=(const MimaPillarPattern&)=delete;
    bool advance();
    unsigned frame() const;
};
}