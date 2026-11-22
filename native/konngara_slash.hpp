#pragma once
#include "konngara_early.hpp"
namespace th01_native {
enum class KonngaraSlashAttack {rain,triangular,aimed};
struct KonngaraSlashHost {
    std::function<void(KonngaraExpression)> expression;
    std::function<void()> face_center;
    std::function<void(unsigned)> slash;
};
class KonngaraSlashPattern {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    KonngaraSlashPattern(KonngaraSlashAttack attack,KonngaraSlashHost host);
    ~KonngaraSlashPattern();
    KonngaraSlashPattern(const KonngaraSlashPattern&)=delete;
    KonngaraSlashPattern& operator=(const KonngaraSlashPattern&)=delete;
    bool advance();
    unsigned frame() const;
    bool face_direction_change_allowed() const;
};
}
