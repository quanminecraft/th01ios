"""Convert checked-in original integer tables and monochrome sprite bitmaps.

Development-time tool only. No trig floats, DOS executable, or emulator used.
"""
import argparse
from pathlib import Path
import re
import struct

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true')
args = parser.parse_args()

def asm_values(name, directive):
    values = []
    for line in (root/'libs/master.lib'/name).read_text(encoding='utf8').splitlines():
        match = re.search(r'\b'+directive+r'\s+([^;]+)', line, re.I)
        if match:
            values.extend(int(value.strip()) for value in match[1].split(','))
    return values

sin = asm_values('sin8[data].asm', 'dw')
atan = asm_values('atan8[data].asm', 'db')
assert len(sin) == 320 and len(atan) == 256
math = ['// Original public-domain master.lib tables by A. Koizuka.',
        '// Values copied from sin8[data].asm and atan8[data].asm, without float regeneration.']
for name, kind, values in [('SinTable8', 'std::int16_t', sin[:256]),
                           ('CosTable8', 'std::int16_t', sin[64:]),
                           ('AtanTable8', 'std::uint8_t', atan)]:
    math.append(f'const {kind} {name}[256] = {{')
    math.extend('\t'+','.join(str(v) for v in values[i:i+16])+',' for i in range(0,256,16))
    math.append('};')

sprites = ['// Derived from original th01/sprites/pellet.bmp, pellet_c.bmp and shape8x8.bmp.']
for file, name, count, width, height in [('pellet.bmp','native_pellet_dots',3,8,8),
                                        ('pellet_c.bmp','native_cloud_dots',2,16,16),
                                        ('shape8x8.bmp','native_shape_dots',7,8,8)]:
    data = (root/'th01/sprites'/file).read_bytes()
    offset, = struct.unpack_from('<I',data,10)
    w,h,planes,bpp,compression = struct.unpack_from('<iiHHI',data,18)
    assert (w,h,planes,bpp,compression) == (count*width,height,1,1,0)
    assert data[54:62] == bytes([0,0,0,0,255,255,255,0])
    stride = (w+31)//32*4
    assert offset+stride*h <= len(data)
    sprites.append(f'const std::uint8_t {name}[{count}][{width*height//8}] = {{')
    for cel in range(count):
        values = []
        for y in range(h):
            for byte in range(width//8):
                values.append(data[offset+(h-1-y)*stride+cel*(width//8)+byte])
        sprites.append('\t{'+','.join(f'0x{v:02x}' for v in values)+'},')
    sprites.append('};')

data = (root/'th01/sprites/laser_s.bmp').read_bytes()
offset, = struct.unpack_from('<I',data,10)
assert struct.unpack_from('<iiHHI',data,18) == (16,64,1,1,0)
assert data[54:62] == bytes([0,0,0,0,255,255,255,0])
assert offset+64*4 <= len(data)
laser_values = [struct.unpack_from('<H',data,offset+(63-y)*4)[0] for y in range(64)]
lasers = ['// Original laser_s.bmp rows in x86 word byte order.',
          'const std::uint16_t native_laser_dots[64] = {']
lasers.extend('\t'+','.join(f'0x{v:04x}' for v in laser_values[i:i+8])+',' for i in range(0,64,8))
lasers.append('};')

# Preserve original Shift-JIS bytes while making the native source independent
# of the compiler's source encoding. Keep only the pause definitions.
pause_source = (root/'th01/shiftjis/hud.hpp').read_bytes().split(b'// Continue menu')[0]
pause = ['// Original pause-menu Shift-JIS literals, escaped from hud.hpp.']
for line in pause_source.splitlines():
    if not line.startswith(b'#define '):
        continue
    def escape_literal(match):
        return b'"'+b''.join(f'\\x{value:02x}'.encode('ascii') for value in match.group(1))+b'"'
    pause.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))

bonus = ['// Original bonus strings and box bitmap, compiler-encoding independent.']
for line in (root/'th01/shiftjis/bonus.hpp').read_bytes().splitlines():
    if line.startswith(b'#define '):
        bonus.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))
data = (root/'th01/sprites/bonusbox.bmp').read_bytes()
offset, = struct.unpack_from('<I',data,10)
assert struct.unpack_from('<iiHHI',data,18) == (8,4,1,1,0)
bonus.append('const unsigned char native_bonus_box[4] = {'+','.join(str(data[offset+(3-y)*4]) for y in range(4))+'};')

routes = ['// Original route-selection Shift-JIS literals.']
for line in (root/'th01/shiftjis/routesel.hpp').read_bytes().splitlines():
    if line.startswith(b'#define '):
        routes.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))

continues = ['// Original Continue menu Shift-JIS literals.']
for line in (root/'th01/shiftjis/hud.hpp').read_bytes().split(b'// Continue menu')[1].splitlines():
    if line.startswith(b'#define '):
        continues.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))

scoredat = ['// Original score-file Shift-JIS literals and little-endian codepoints.']
for file in ['scoredat.hpp','fns.hpp']:
    for line in (root/'th01/shiftjis'/file).read_bytes().splitlines():
        if line.startswith(b'#define ') and (file=='scoredat.hpp' or line.startswith(b'#define SCOREDAT_FN_')):
            scoredat.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))
        match = re.match(rb"static const shiftjis_kanji_t (SCOREDAT_ROUTE_\w+)\s*=\s*'(.{2})';",line)
        if match:
            scoredat.append('static const shiftjis_kanji_t '+match[1].decode('ascii')+' = '+str(int.from_bytes(match[2],'little'))+';')

regist = ['// Original high-score UI literals and little-endian Shift-JIS kanji.']
for file in ['title.hpp','regist.hpp']:
    for line in (root/'th01/shiftjis'/file).read_bytes().splitlines():
        if line.startswith(b'#include '):
            continue
        line = re.sub(rb'"([^"\r\n]*)"',escape_literal,line)
        line = re.sub(rb"'([^'\r\n]{2})'",lambda m: str(int.from_bytes(m[1],'little')).encode('ascii'),line)
        regist.append(line.decode('ascii'))

entrance_jis = ['// Original six entrance characters from th01/shiftjis/entrance.hpp.', 'constexpr unsigned entrance_jis[] = {']
for raw in re.findall(rb"touhou_reiiden_animate_char\('(.{2})'\);",(root/'th01/shiftjis/entrance.hpp').read_bytes()):
    lead,trail=raw
    row=((lead-0x81) if lead<0xa0 else (lead-0xc1))*2+0x21
    if trail>=0x9f:
        row+=1;col=trail-0x7e
    else:
        col=trail-(0x1f if trail<0x7f else 0x20)
    entrance_jis.append(f'0x{row:02x}{col:02x},')
entrance_jis.append('};')

title_strings = ['// Original title/options/music Shift-JIS literals.']
for line in (root/'th01/shiftjis/op.hpp').read_bytes().splitlines():
    if not line.startswith(b'#include '):
        title_strings.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))
music_files = (root/'th01/shiftjis/fns.hpp').read_bytes().split(b'#define MUSIC_FILES')[1].split(b'#define SCOREDAT_FN')[0]
title_strings.extend((b'#define MUSIC_FILES'+music_files).decode('ascii').splitlines())

ending_verdict = ['// Original ending/verdict and title literals, encoding independent.']
for file in ['end.hpp','title.hpp']:
    for line in (root/'th01/shiftjis'/file).read_bytes().splitlines():
        if line.startswith(b'#include '):
            continue
        ending_verdict.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))

ending_captions = ['// Original ending and boss slideshow caption literals.']
for line in (root/'th01/shiftjis/end.hpp').read_bytes().split(b'// Verdict screen')[0].splitlines():
    if line.startswith(b'#define '):
        ending_captions.append(re.sub(rb'"([^"\r\n]*)"',escape_literal,line).decode('ascii'))

pillar = ['// Original th01/sprites/pillar.bmp, little-endian planar row words.',
          'const std::uint32_t native_pillar_rows[2][8] = {']
data = (root/'th01/sprites/pillar.bmp').read_bytes()
offset, = struct.unpack_from('<I', data, 10)
assert struct.unpack_from('<iiHHI', data, 18) == (64,8,1,1,0)
for cel in range(2):
    rows = [int.from_bytes(data[offset+(7-y)*8+cel*4:offset+(7-y)*8+cel*4+4], 'little') for y in range(8)]
    pillar.append('\t{'+','.join(f'0x{row:08x}u' for row in rows)+'},')
pillar.append('};')
for name, lines in [('math_tables.inc',math),('pellet_sprites.inc',sprites),('laser_sprites.inc',lasers),('pillar_sprites.inc',pillar),('pause_strings.inc',pause),('bonus_strings.inc',bonus),('route_strings.inc',routes),('continue_strings.inc',continues),('scoredat_strings.inc',scoredat),('regist_strings.inc',regist),('ending_captions.inc',ending_captions),('ending_verdict_strings.inc',ending_verdict),('title_strings.inc',title_strings),('entrance_jis.inc',entrance_jis)]:
    path = root/'native'/name
    output = ('\n'.join(lines)+'\n').encode('utf8')
    if args.check:
        if not path.exists() or path.read_bytes() != output:
            raise SystemExit(f'Stale generated data: {path}')
    elif not path.exists() or path.read_bytes() != output:
        path.write_bytes(output)
print('Original integer-table/sprite conversion checked' if args.check else 'Generated original integer tables and pellet sprites')
