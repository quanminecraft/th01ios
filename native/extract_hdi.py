"""Read-only extraction of a PC-98 Anex86 HDI FAT12/16 partition.

No boot code is executed. All output is a separate directory under build/.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('image', type=Path)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
out = a.output.resolve()
if not out.is_relative_to(ROOT / 'build') or out.exists():
    p.error('Choose a new directory under build/')
disk = a.image.read_bytes()
_, _, header, image_bytes, physical_sector, sectors, heads, cylinders = struct.unpack_from('<8I', disk)
if header < 32 or physical_sector not in (256, 512, 1024) or header + image_bytes != len(disk):
    raise ValueError('Invalid Anex86 HDI header/extent')
partitions = disk[header + physical_sector: header + physical_sector + 512]
entry = next((partitions[i:i+32] for i in range(0, 512, 32)
              if partitions[i] & 0x7f == 0x21 and partitions[i+1] & 0x7f == 1), None)
if entry is None:
    raise ValueError('No PC-98 MS-DOS partition')
start_sector, start_head, start_cylinder = struct.unpack_from('<BBH', entry, 8)
boot = header + ((start_cylinder * heads + start_head) * sectors + start_sector) * physical_sector
bps, spc, reserved, fats, root_count, total, media, fat_sectors = struct.unpack_from('<HBHBHHBH', disk, boot+11)
if not total:
    total = struct.unpack_from('<I', disk, boot+32)[0]
root_sectors = (root_count*32 + bps-1)//bps
data_sector = reserved + fats*fat_sectors + root_sectors
cluster_count = (total-data_sector)//spc
if bps not in (512, 1024, 2048) or not spc or not fats or not 0 < cluster_count < 65525:
    raise ValueError('Invalid/unsupported FAT BPB')
fat12 = cluster_count < 4085
if boot+total*bps > len(disk):
    raise ValueError('Filesystem exceeds disk extent')
fat = disk[boot+reserved*bps:boot+(reserved+fat_sectors)*bps]
cluster_size = bps*spc

def chain(cluster):
    chunks, seen = [], set()
    while cluster < (0xff8 if fat12 else 0xfff8):
        if not 2 <= cluster < cluster_count+2 or cluster in seen:
            raise ValueError('Invalid/cyclic FAT chain')
        seen.add(cluster)
        offset = boot+data_sector*bps+(cluster-2)*cluster_size
        chunks.append(disk[offset:offset+cluster_size])
        index = cluster*3//2 if fat12 else cluster*2
        value = struct.unpack_from('<H', fat, index)[0]
        cluster = ((value>>4) if cluster & 1 else value & 0xfff) if fat12 else value
    return b''.join(chunks)

manifest = []
def directory(raw, relative):
    for offset in range(0, len(raw), 32):
        item = raw[offset:offset+32]
        if item[0] == 0:
            break
        if item[0] == 0xe5 or item[11] & 8 or item[11] == 0x0f:
            continue
        name = item[:8].rstrip(b' ')
        extension = item[8:11].rstrip(b' ')
        name = (name + (b'.'+extension if extension else b'')).decode('cp932')
        if name in ('.', '..'):
            continue
        if any(x in name for x in ('/', '\\', ':')):
            raise ValueError('Unsafe FAT filename')
        path = relative / name
        target = out / path
        cluster = struct.unpack_from('<H', item, 26)[0]
        size = struct.unpack_from('<I', item, 28)[0]
        if item[11] & 0x10:
            target.mkdir(parents=True)
            directory(chain(cluster), path)
        else:
            content = chain(cluster) if size else b''
            if len(content) < size:
                raise ValueError('Short FAT file')
            content = content[:size]
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(content)
            manifest.append(dict(path=str(path), bytes=size, sha256=hashlib.sha256(content).hexdigest()))

out.mkdir(parents=True)
root = boot + (reserved+fats*fat_sectors)*bps
directory(disk[root:root+root_count*32], Path())
(out/'extraction.json').write_text(json.dumps(dict(imageSha256=hashlib.sha256(disk).hexdigest(),
    filesystem='FAT12' if fat12 else 'FAT16', bytesPerSector=bps, files=manifest), indent=2, ensure_ascii=False), encoding='utf-8')
print('Extracted', len(manifest), 'files,', 'FAT12' if fat12 else 'FAT16', 'to', out)
