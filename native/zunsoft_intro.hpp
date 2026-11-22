#pragma once
#include "framebuffer.hpp"
#include "input_backend.hpp"
#include "math_backend.hpp"
namespace th01_native {
inline SpriteSet read_zunsoft_logo(const Bytes& bytes) {
 if(bytes.size()<80 || std::string(bytes.begin(),bytes.begin()+5)!="BFNT\x1a")throw std::runtime_error("Invalid ZUN Soft BFNT");
 auto word=[&](unsigned at){return unsigned(bytes[at])+(unsigned(bytes[at+1])<<8);};
 if(bytes[5]!=0x83 || word(8)!=64 || word(10)!=16 || word(12)!=0 || word(14)!=15 || word(28)!=0 || bytes.size()!=80+16*512)
  throw std::runtime_error("Unsupported ZUN Soft BFNT geometry");
 SpriteSet out;
 for(unsigned i=0;i<16;++i) {
  auto at=32+i*3; // BFNT stores high-nibble B,R,G palette components.
  out.palette[i]=0xff000000u|((bytes[at+1]>>4)*17u<<16)|((bytes[at+2]>>4)*17u<<8)|(bytes[at]>>4)*17u;
  Sprite image{64,16,Bytes(1024),Bytes(1024)};
  for(unsigned pixel=0;pixel<1024;++pixel) {
   auto packed=bytes[80+i*512+pixel/2];auto color=(pixel&1)?packed&15:packed>>4;
   image.color[pixel]=color;image.opaque[pixel]=color!=0;
  }
  out.images.push_back(std::move(image));
 }
 return out;
}
// Native th01/zunsoft.cpp animation; no DOS program is executed.
class ZunSoftIntro {
 struct Point {int x,y;};
 SpriteSet logo;Framebuffer fb;
 std::array<Point,4> circles{{{128,320},{256,240},{384,160},{512,80}}};
 std::array<Point,4> velocity{{{-8,8},{-8,8},{-8,8},{-8,8}}};
 std::array<Point,50> stars{};std::array<unsigned,50> speed{};
 std::uint32_t rng=1;
 unsigned frame=0,pattern=0,star_angle=0x40,wave_phase=0;
 int tone=0,wave_length=23,wave_amplitude=0;
 bool done=false,skipped_input=false;
 static constexpr int sine7[256]={
#include "zunsoft_sine.inc"
 };
 unsigned random(){rng=rng*0x015a4e35u+1;return (rng>>16)&0x7fff;}
 static int floor_div(int n,int d){return n>=0?n/d:-((-n+d-1)/d);}
 void point(int x,int y,unsigned color){if(x>=96&&x<=543&&y>=100&&y<=299)fb.put(0,x,y,color);}
 void span(int left,int right,int y,unsigned color){
  if(y<100||y>299)return;
  for(int x=std::max(left,96);x<=std::min(right,543);++x)fb.put(0,x,y,color);
 }
 void disc(Point center,unsigned color){
  int x=96,y=0,error=96;
  do{
   span(center.x-x,center.x+x,center.y-y,color);span(center.x-x,center.x+x,center.y+y,color);
   error-=y*2+1;
   if(error<0){span(center.x-y,center.x+y,center.y-x,color);span(center.x-y,center.x+y,center.y+x,color);--x;error+=x*2;}
   ++y;
  }while(x>=y);
 }
 void put_logo(unsigned image,int left,bool wave){
  const auto& sprite=logo.images.at(image);
  for(unsigned y=0;y<16;++y){
   const int xrow=wave?left+floor_div(wave_amplitude*sine7[((y*256)/wave_length+wave_phase)&255],128):left;
   for(unsigned x=0;x<64;++x)if(sprite.opaque[y*64+x])point(xrow+x,192+y,sprite.color[y*64+x]);
  }
 }
public:
 explicit ZunSoftIntro(const Bytes& data):logo(read_zunsoft_logo(data)){
  fb.palette.fill(0xff000000u);
  for(unsigned i=0;i<50;++i){stars[i]={int(random()%640),int(random()%400)};speed[i]=random()%32+6;}
 }
 bool step(const InputKeys& keys={}){
  if(done)return false;
  if(any_menu_input(keys)){done=skipped_input=true;return false;}
  if(frame>180&&tone<=0){done=true;return false;}
  if(frame>180)tone-=2;else if(tone<100)tone+=2;
  for(unsigned i=0;i<16;++i){
   auto color=0xff000000u;
   for(unsigned shift:{0u,8u,16u})color|=((((logo.palette[i]>>shift)&255)/17)*tone/100)*17u<<shift;
   fb.palette[i]=color;
  }
  for(auto& plane:fb.pages[0])for(unsigned y=100;y<=299;++y)std::fill_n(plane.begin()+y*80+12,56,0);
  for(unsigned i=0;i<50;++i){
   point(stars[i].x,stars[i].y,5);
   stars[i].x=(stars[i].x+floor_div(int(speed[i])*Cos8(star_angle),256)+640)%640;
   stars[i].y=(stars[i].y+floor_div(int(speed[i])*Sin8(star_angle),256)+400)%400;
  }
  star_angle=(star_angle+1)&255;
  for(int i=3;i>=0;--i){
   disc(circles[i],4-i);circles[i].x+=velocity[i].x;circles[i].y+=velocity[i].y;
   if(circles[i].x<=32||circles[i].x>607)velocity[i].x=-velocity[i].x;
   if(circles[i].y<=32||circles[i].y>367)velocity[i].y=-velocity[i].y;
  }
  if(frame>=50&&frame<170){
   if(frame==55||frame==60||frame==65||frame==110||frame==155||frame==160||frame==165)pattern+=2;
   const bool wave=frame>=90&&frame<130;put_logo(pattern,256,wave);put_logo(pattern+1,320,wave);
   if(frame>=90&&frame<110){--wave_length;wave_phase=(wave_phase+4)&255;wave_amplitude+=4;}
   else if(frame>=110&&frame<130){++wave_length;wave_phase=(wave_phase+4)&255;wave_amplitude-=4;}
  }
  ++frame;return true;
 }
 const Framebuffer& framebuffer() const{return fb;}
 unsigned frames() const{return frame;}
 unsigned logo_pattern() const{return pattern;}
 bool skipped() const{return skipped_input;}
};
}
