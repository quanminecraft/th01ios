"""Prepare a host-only MDRV98 reference player, not part of the native port.

Run the resulting reference.conf with the user's DOSBox-X executable. DX-CAPTURE
/A records PLAY.COM. Only copies of the original driver/MDT/fonts are used.
"""
from pathlib import Path
import shutil
import sys

assets, output = map(Path, sys.argv[1:3])
seconds = int(sys.argv[3]) if len(sys.argv) > 3 else 20
assert 1 <= seconds <= 59
output = output.resolve()
output.mkdir(parents=True, exist_ok=True)
(output / 'captures').mkdir(exist_ok=True)
for name in ['MDRV98.COM', 'FONT.ROM'] + [p.name for p in assets.glob('2608_*.WAV')]:
    shutil.copyfile(assets / name, output / name)

# AH=09 detect, AH=06 DS:SI MSET, AH=00 MPLAY (ReC98 th01/snd/mdrv2.cpp).
code = bytearray.fromhex('b409cdf2be0000b406cdf231c0cdf2b42ccd2188f3')
loop = len(code)
# DOS AH=2C wall-clock seconds, wrap mod 60; wait the requested interval.
code += bytes.fromhex('b42ccd2188f028d87302043c3c') + bytes([seconds])
branch = len(code)
code += bytes([0x72, (loop - (branch + 2)) & 255])
code += bytes.fromhex('b403cdf2b8004ccd21')
code[5:7] = (0x100 + len(code)).to_bytes(2, 'little')
assert code[branch] == 0x72 and branch + 2 + int.from_bytes(code[branch+1:branch+2], 'little', signed=True) == loop
(output / 'PLAY.COM').write_bytes(code + (assets / 'REIMU.MDT').read_bytes())
config = f'''[sdl]
fullscreen=false
output=surface
[dosbox]
machine=pc98
captures={output / 'captures'}
show recorded filename=false
[pc98]
pc-98 fm board=board86
[cpu]
cycles=fixed 20000
[mixer]
rate=48000
[autoexec]
mount c "{output}"
c:
mdrv98 -M16 > DRIVER.TXT
dx-capture /a PLAY.COM
exit
'''
(output / 'reference.conf').write_text(config, encoding='utf-8')
print(f'Reference prepared: {seconds}s, {len(code)}-byte playback stub')
