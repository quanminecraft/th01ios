#include "konngara_face.hpp"
#include "boss_entity_backend.hpp"
namespace {
using face_direction_t=std::int16_t;using face_expression_t=std::int16_t;
constexpr int FD_CENTER=2,FD_COUNT=3,FE_NEUTRAL=0,FE_CLOSED=1,FE_AIM=3;
struct Runtime {
    CBossEntity head{},closed_glare{},aim{};
    face_direction_t direction=FD_CENTER;face_expression_t expression=FE_NEUTRAL;
    bool16 allowed=true;
};
Runtime* active=nullptr;
Runtime& current() {if(!active) throw std::logic_error("Konngara face is not bound");return *active;}
void head_put(int direction) {current().head.put_8(280,80,direction);}
void face_aim_put(int direction) {current().aim.put_8(280,128,direction);}
void face_put(int expression,int direction) {current().closed_glare.put_8(280,128,(expression-FE_CLOSED)*FD_COUNT+direction);}
#define face_direction current().direction
#define face_expression current().expression
#define face_direction_can_change current().allowed
#include "th01/main/boss/konngara_face.inc"
#undef face_direction
#undef face_expression
#undef face_direction_can_change
}
namespace th01_native {
struct KonngaraFace::Impl {
    Runtime runtime;
    Impl() {
        if(active) throw std::logic_error("Konngara face supports one active instance");
        try {runtime.head.load("boss8_1.bos",0);runtime.closed_glare.load("boss8_e1.bos",1);runtime.aim.load("boss8_e2.bos",2);}catch(...) {for(int i=0;i<3;++i)bos_entity_free(i);throw;}
        active=&runtime;
    }
    ~Impl() {for(int i=0;i<3;++i)bos_entity_free(i);active=nullptr;}
};
KonngaraFace::KonngaraFace():impl(std::make_unique<Impl>()) {}
KonngaraFace::~KonngaraFace()=default;
void KonngaraFace::direction(KonngaraDirection value) {int n=static_cast<int>(value);if(n<0 || n>2)throw std::invalid_argument("Konngara face direction");face_direction_set_and_put(n);}
void KonngaraFace::expression(KonngaraExpression value) {int n=static_cast<int>(value);if(n<0 || n>3)throw std::invalid_argument("Konngara expression");face_expression_set_and_put(n);}
void KonngaraFace::tracking(int left) {direction(left<198 ? KonngaraDirection::left : left>396 ? KonngaraDirection::right : KonngaraDirection::center);}
void KonngaraFace::allow_direction_change(bool value) {impl->runtime.allowed=value;}
KonngaraDirection KonngaraFace::direction() const {return static_cast<KonngaraDirection>(impl->runtime.direction);}
KonngaraExpression KonngaraFace::expression() const {return static_cast<KonngaraExpression>(impl->runtime.expression);}
}
