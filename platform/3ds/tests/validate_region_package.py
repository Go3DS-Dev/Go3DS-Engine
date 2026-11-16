"""Validate Milestone A's embedded PCK and preservation of earlier binaries."""
from pathlib import Path
import hashlib
import json
import struct

repo = Path(__file__).resolve().parents[3]
platform = repo / "platform/3ds"
pack = (platform / "romfs/Game.pck").read_bytes()
assert struct.unpack_from("<4s5IQ", pack) == (b"GDPC", 2, 4, 3, 0, 0, 0)
assert struct.unpack_from("<I", pack, 96)[0] == 5
entries, spans, pos = {}, [], 100
for _ in range(5):
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
assert spans[0][0] == pos and spans[-1][1] == len(pack)
assert all(a[1] == b[0] for a, b in zip(spans, spans[1:]))
for name in ("project.godot", "main.tscn", "main.gd", "atlas.png"):
    assert entries["res://" + name] == (platform / "region_test_project" / name).read_bytes()
assert entries["res://.godot/global_script_class_cache.cfg"] == b"list=[]\n"
assert struct.unpack_from(">II", entries["res://atlas.png"], 16) == (96, 48)
binary = repo / "bin/godot-regions-milestone-a-20261116.3dsx"
data = binary.read_bytes()
assert data[:4] == b"3DSX"
smdh_offset, smdh_size, romfs_offset = struct.unpack_from("<III", data, 32)
assert data[smdh_offset:smdh_offset + 4] == b"SMDH"
assert 0 < smdh_size and smdh_offset + smdh_size <= romfs_offset < len(data)
assert pack in data[romfs_offset:]
assert b"FPS %.1f C3D CPU/GPU" in data
assert b"3DS: non-finite texture region; skipping." in data
preserved = {
    "godot-2d-checkpoint-20261116.3dsx": "96d1af9a03c66011a935de97c3990e9b83e3ed92407fc579a5cff1cc542a89b5",
    "godot-sprite2d-citro2d.3dsx": "f557a4813ab7d6d45e7211f048fa60a7dc0e9a4205600240de7093714b698ab4",
}
manifest = repo.parent / "work/regions-preserved-builds.json"
if manifest.exists():
    preserved.update(json.loads(manifest.read_text()))
for name, digest in preserved.items():
    assert hashlib.sha256((repo / "bin" / name).read_bytes()).hexdigest() == digest
print("PACKAGE PASS: PCK entries/checksums/source match, atlas dimensions, cache, 3DSX/SMDH/RomFS, renderer markers, preserved binaries.")
print(binary)
print("Bytes:", len(data))
print("SHA256:", hashlib.sha256(data).hexdigest())
