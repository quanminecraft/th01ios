"""Audit shared Sariel/transition bodies against the original DOS sources."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parent.parent
for source,includes in (
    ('th01/main/boss/b20m.cpp',list((root/'th01/main/boss').glob('sariel_*.inc'))),
    ('th01/hardware/pgtrans.cpp',[root/'th01/hardware/pgtrans_diagonal.inc']),
    ('th01/hardware/grcg8x8m.cpp',[root/'th01/hardware/grcg8x8m_body.inc']),
):
    body=(root/source).read_bytes()
    for _ in range(10):
        before=body
        for inc in includes:
            relative=inc.relative_to(root).as_posix().encode()
            body=body.replace(b'#include "'+relative+b'"',inc.read_bytes())
        if body==before:
            break
    else:
        raise AssertionError('Nested shared includes did not converge')
    visible=True
    stack=[]
    output=[]
    for line in body.splitlines(keepends=True):
        directive=line.strip()
        if directive in (b'#ifdef TH01_NATIVE_ORB_PHYSICS',b'#ifndef TH01_NATIVE_ORB_PHYSICS'):
            condition=directive.startswith(b'#ifndef');stack.append((visible,condition));visible=visible and condition
        elif directive==b'#else' and stack:
            outer,condition=stack[-1];visible=outer and not condition
        elif directive==b'#endif' and stack:
            visible=stack.pop()[0]
        elif visible:
            output.append(line)
    assert not stack
    body=b''.join(output)
    body=body.replace(b'typedef int sariel_frame_t;',b'').replace(b'sariel_frame_t',b'int')
    original=subprocess.check_output(['git','show','HEAD:'+source],cwd=root)
    assert b''.join(body.split())==b''.join(original.split()),source+' differs from original'
    print('Expanded DOS source matches HEAD:',source)
p=(root/'th01/sprites/ileave_m.bmp').read_bytes()
assert [[p[62+(7-y)*12+i] for y in range(8)] for i in (1,2)]==[[0x55,0xaa]*4,[0xaa,0x55]*4]
print('Native transition masks match original interleave bitmap')
import struct,re
leaf_source=(root/'native/leaf_sprites.inc').read_text()
rows=re.findall(r'\{\{\{([0-9,]+)\}\}\}',leaf_source)
assert len(rows)==3
for name,actual in zip(('leaf_s','leaf_l','leaf_r'),rows):
    data=(root/'th01/sprites'/f'{name}.bmp').read_bytes()
    off=struct.unpack_from('<I',data,10)[0]
    assert struct.unpack_from('<iiHHI',data,18)==(8,8,1,1,0)
    assert list(map(int,actual.split(',')))==[data[off+(7-y)*4] for y in range(8)]
print('Native leaf sprites match original bitmaps')
