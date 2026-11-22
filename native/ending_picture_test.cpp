#include "ending_picture.hpp"
#include <iostream>
#include <memory>

int main() {try {
 using namespace th01_native;
 auto fb=std::make_unique<Framebuffer>();
 for(int y=0;y<400;++y)for(int x=0;x<640;++x)
  fb->put(1,x,y,((x/320)+2*(y/200)+x/7+y/11)&15);
 auto source=fb->pages[1];
 auto palette=fb->palette;
 for(unsigned q=0;q<4;++q) {
  fb->clear(0,9);
  ending_picture_show(*fb,q);
  for(int y=0;y<400;++y)for(int x=0;x<640;++x) {
   auto expected=(x>=160&&x<480&&y>=100&&y<300)
    ? fb->pixel(1,(q%2)*320+x-160,(q/2)*200+y-100):9;
   if(fb->pixel(0,x,y)!=expected)throw std::runtime_error("Ending quarter raster mismatch");
  }
  if(source!=fb->pages[1]||palette!=fb->palette)throw std::runtime_error("Ending copy changed source or palette");
 }
 auto before=fb->pages;
 bool rejected=false;
 try {ending_picture_show(*fb,4);}catch(const std::out_of_range&) {rejected=true;}
 if(!rejected||before!=fb->pages)throw std::runtime_error("Invalid quarter mutated framebuffer");
 std::cout<<"All four original ending picture copies, borders, source and palette passed\n";
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}
