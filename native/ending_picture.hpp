#pragma once
#include "framebuffer.hpp"

namespace th01_native {
// Copy one original 320x200 ending quarter from page 1 to the centered
// picture on page 0. Pixels outside the picture and the palette stay intact.
void ending_picture_show(Framebuffer& framebuffer, unsigned quarter);
}
