#pragma once
#include "assets.hpp"
#include <memory>
#include <functional>
namespace th01_native {
struct ElisEntityState{int left,top,cel;bool hittable;};
struct ElisFinalState{bool bat;int frame,subphase,bat_pattern,human_pattern;};
class ElisTeleport{
    struct Impl;std::unique_ptr<Impl> impl;
public:
    explicit ElisTeleport(std::uint32_t seed=1);
    ~ElisTeleport();
    ElisTeleport(const ElisTeleport&)=delete;
    ElisTeleport& operator=(const ElisTeleport&)=delete;
    void entrance(const std::vector<Entry>& archive,std::function<void(unsigned)> wait);
    void begin_final_phase();
    void advance_final_phase();
    ElisFinalState final_state()const;
    void begin_phase3(unsigned attack);
    bool advance_phase3();
    void begin_star();
    bool advance_star();
    void set_delay(std::function<void(unsigned)> delay);
    void begin_human_pattern(unsigned pattern);
    bool advance_human_pattern();
    void fire_bat_pattern(unsigned pattern,unsigned tick);
    void begin_flight(unsigned pattern=0);
    bool advance_flight();
    void begin_transform(bool to_bat);
    bool advance_transform();
    void begin(int left,int top);
    bool advance();
    int frame()const;
    ElisEntityState girl()const;
    ElisEntityState bat()const;
};
}