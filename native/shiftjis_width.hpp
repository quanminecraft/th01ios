#pragma once
// Byte-based width matches the original strings used here: one-byte ASCII
// cells are 8 pixels and each two-byte fullwidth code point is 16 pixels.
constexpr unsigned shiftjis_w(const char* s) {return *s ? 8+shiftjis_w(s+1) : 0;}
