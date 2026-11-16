"""Independently verify the PCK directory and the delivered 3DSX payload."""
from pathlib import Path
import hashlib
import struct

repo = Path(__file__).resolve().parents[3]
platform = repo / "platform/3ds"
pack = (platform / "romfs/Game.pck").read_bytes()
assert struct.unpack_from("<4s5IQ", pack) == (b"GDPC", 2, 4, 3, 0, 0, 0)
count, = struct.unpack_from("<I", pack, 96)
assert count == 5
pos = 100
entries = {}
spans = []
for _ in range(count):
    length, = struct.unpack_from("<I", pack, pos)
    pos += 4
    name = pack[pos:pos + length].rstrip(b"\0").decode()
    pos += length
    offset, size, digest, flags = struct.unpack_from("<QQ16sI", pack, pos)
    pos += 36
    assert flags == 0 and offset + size <= len(pack)
    data = pack[offset:offset + size]
    assert hashlib.md5(data).digest() == digest
    assert name not in entries
    entries[name] = data
    spans.append((offset, offset + size))
assert sorted(spans)[0][0] == pos
assert all(a[1] == b[0] for a, b in zip(sorted(spans), sorted(spans)[1:]))
assert sorted(spans)[-1][1] == len(pack)
for name in ("project.godot", "main.tscn", "main.gd", "sprite.png"):
    assert entries["res://" + name] == (platform / "test_project" / name).read_bytes()
assert entries["res://.godot/global_script_class_cache.cfg"] == b"list=[]\n"
binary = repo / "bin/godot-2d-checkpoint-20261116.3dsx"
data = binary.read_bytes()
assert data[:4] == b"3DSX"
smdh_offset, smdh_size, romfs_offset = struct.unpack_from("<III", data, 32)
assert data[smdh_offset:smdh_offset + 4] == b"SMDH"
assert smdh_size > 0 and smdh_offset + smdh_size <= romfs_offset < len(data)
assert pack in data[romfs_offset:]
assert b"FPS %.1f C3D CPU/GPU" in data
working = (repo / "bin/godot-sprite2d-citro2d.3dsx").read_bytes()
assert hashlib.sha256(working).hexdigest() == "f557a4813ab7d6d45e7211f048fa60a7dc0e9a4205600240de7093714b698ab4"
print("PACKAGE PASS: 5 entries, checksums, source match, cache, 3DSX/SMDH/RomFS, diagnostics, preserved working binary.")
print(binary)
print("Bytes:", len(data))
print("SHA256:", hashlib.sha256(data).hexdigest())
