"""Verify Elis shared helper extraction preserves the original DOS branch."""
from pathlib import Path
import subprocess,re
root=Path(__file__).resolve().parent.parent
p=root/'th01/main/boss/b15m.cpp'
expanded=p.read_bytes()
for inc in p.parent.glob('elis_*.inc'):
    expanded=expanded.replace(b'#include "th01/main/boss/'+inc.name.encode()+b'"',inc.read_bytes())
visible=True
stack=[]
output=[]
for line in expanded.splitlines(keepends=True):
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
normalized=b''.join(output)
normalized=re.sub(rb'typedef int elis_int_t;\s*',b'',normalized).replace(b'elis_int_t',b'int')
output=[normalized]
original=subprocess.check_output(['git','show','HEAD:th01/main/boss/b15m.cpp'],cwd=root)
assert b''.join(b''.join(output).split())==b''.join(original.split()),'DOS Elis source differs from original'
print('Expanded DOS Elis source matches HEAD')
p=root/'th01/main/boss/entity_a.cpp'
expanded=p.read_bytes()
for name in ('entity_motion.inc','entity_wave.inc'):
    expanded=expanded.replace(b'#include "th01/main/boss/'+name.encode()+b'"',(p.parent/name).read_bytes())
original=subprocess.check_output(['git','show','HEAD:th01/main/boss/entity_a.cpp'],cwd=root)
assert b''.join(expanded.split())==b''.join(original.split()),'Shared entity source differs from original'
print('Expanded shared entity motion/wave source matches HEAD')