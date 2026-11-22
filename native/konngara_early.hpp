#pragma once
#include <functional>
#include <memory>
namespace th01_native {
enum class KonngaraEarlyAttack {diamond_cross_rain,cup_spreads,two_snakes_spreads,four_snakes,aimed_rows,cup_spray,rain_edges,lasers_spread,sleeve_rain};
enum class KonngaraExpression {neutral=0,closed=1,glare=2,aim=3};
// Runs one original attack; caller updates existing projectile actors after it.
class KonngaraEarlyPattern {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    KonngaraEarlyPattern(KonngaraEarlyAttack attack,std::function<void(KonngaraExpression)> expression);
    ~KonngaraEarlyPattern();
    KonngaraEarlyPattern(const KonngaraEarlyPattern&)=delete;
    KonngaraEarlyPattern& operator=(const KonngaraEarlyPattern&)=delete;
    bool advance();
    unsigned frame() const;
};
}
