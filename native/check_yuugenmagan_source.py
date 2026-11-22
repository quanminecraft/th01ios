"""Verify shared YuugenMagan bodies preserve the original DOS code path."""
from pathlib import Path
import subprocess,re
root=Path(__file__).resolve().parent.parent
p=root/'th01/main/boss/b10m.cpp'
expanded=p.read_bytes()
for inc in p.parent.glob('yuugenmagan_*.inc'):
    expanded=expanded.replace(b'#include "th01/main/boss/'+inc.name.encode()+b'"',inc.read_bytes())
expanded=expanded.replace(b'#include "th01/main/boss/yuugenmagan_types.hpp"',(p.parent/'yuugenmagan_types.hpp').read_bytes())
visible=True
stack=[]
output=[]
for line in expanded.splitlines(keepends=True):
    directive=line.strip()
    if directive in (b'#ifdef TH01_NATIVE_ORB_PHYSICS',b'#ifndef TH01_NATIVE_ORB_PHYSICS'):
        condition=directive.startswith(b'#ifndef')
        stack.append((visible,condition));visible=visible and condition
    elif directive==b'#else' and stack:
        outer,condition=stack[-1];visible=outer and not condition
    elif directive==b'#endif' and stack:
        visible=stack.pop()[0]
    elif visible:
        output.append(line)
assert not stack
expanded=b''.join(output)
expanded=re.sub(rb'typedef int yuugenmagan_int_t;\s*',b'',expanded)
expanded=expanded.replace(b'yuugenmagan_int_t',b'int')
expanded=re.sub(rb'#define yuugenmagan_component[^\n]*\n',b'',expanded)
expanded=re.sub(rb'yuugenmagan_component\((z_Palettes|stage_palette)\[COL_YOKOSHIMA\],\s*(\w+)\)',rb'\1[COL_YOKOSHIMA].v[\2]',expanded)
# Header-only metadata and its explanation are not executable DOS source.
expanded=re.sub(rb'#pragma once[^\n]*\n',b'',expanded)
expanded=re.sub(rb'// Turbo C\+\+ uses 16-bit int; shared boss state must retain that width\.[^\n]*\n',b'',expanded)
original=subprocess.check_output(['git','show','HEAD:th01/main/boss/b10m.cpp'],cwd=root)
assert b''.join(expanded.split())==b''.join(original.split()),'DOS YuugenMagan branch differs from original'
print('Expanded DOS YuugenMagan matches HEAD after type/component alias normalization')