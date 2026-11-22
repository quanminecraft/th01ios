#pragma once
#include "konngara_early.hpp"
namespace th01_native {
enum class KonngaraDirection {right=0,left=1,center=2};
// Caller keeps BossEntityBinding and ShotBackendBinding alive around this
// controller. Its three BOS banks occupy original slots 0,1,2.
class KonngaraFace {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    KonngaraFace();
    ~KonngaraFace();
    KonngaraFace(const KonngaraFace&)=delete;
    KonngaraFace& operator=(const KonngaraFace&)=delete;
    void direction(KonngaraDirection value);
    void expression(KonngaraExpression value);
    void tracking(int player_left);
    void allow_direction_change(bool value);
    KonngaraDirection direction() const;
    KonngaraExpression expression() const;
};
}
