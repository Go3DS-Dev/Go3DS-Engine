"""Check a named 3DSX against a selected fixture, independently of current RomFS.
Usage: python validate_embedded_project.py <binary filename> <project directory name>
"""
from pathlib import Path
import hashlib
import json
import struct
import sys

repo = Path(__file__).resolve().parents[3]
binary = repo / "bin" / sys.argv[1]
project = repo / "platform/3ds" / sys.argv[2]
data = binary.read_bytes()
assert data[:4] == b"3DSX"
smdh_offset, smdh_size, romfs_offset = struct.unpack_from("<III", data, 32)
assert data[smdh_offset:smdh_offset + 4] == b"SMDH"
assert smdh_size > 0 and smdh_offset + smdh_size <= romfs_offset < len(data)
start = data.index(b"GDPC" + struct.pack("<I", 2), romfs_offset)
pack = data[start:]
assert struct.unpack_from("<4s5IQ", pack) == (b"GDPC", 2, 4, 3, 0, 0, 0)
count, = struct.unpack_from("<I", pack, 96)
entries, spans, pos = {}, [], 100
for _ in range(count):
    length, = struct.unpack_from("<I", pack, pos)
    pos += 4
    name = pack[pos:pos + length].rstrip(b"\0").decode()
    pos += length
    offset, size, digest, flags = struct.unpack_from("<QQ16sI", pack, pos)
    pos += 36
    assert flags == 0 and offset + size <= len(pack) and name not in entries
    entries[name] = pack[offset:offset + size]
    assert hashlib.md5(entries[name]).digest() == digest
    spans.append((offset, offset + size))
assert spans[0][0] == pos
assert all(a[1] == b[0] for a, b in zip(spans, spans[1:]))
expected = {"res://" + p.name: p.read_bytes() for p in project.iterdir()
            if p.suffix in (".gd", ".tscn", ".png", ".godot")}
expected["res://.godot/global_script_class_cache.cfg"] = b"list=[]\n"
assert entries == expected
assert b"FPS %.1f C3D CPU/GPU" in data
manifest = repo.parent / "work/primitives-preserved-builds.json"
for name, digest in json.loads(manifest.read_text()).items():
    assert hashlib.sha256((repo / "bin" / name).read_bytes()).hexdigest() == digest
print("PASS: embedded files/checksums/cache, 3DSX/SMDH, diagnostics, all previous named binaries unchanged.")
print(binary)
print("Bytes:", len(data), "SHA256:", hashlib.sha256(data).hexdigest())
