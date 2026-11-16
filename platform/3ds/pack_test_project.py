"""Create a tiny PNG fixture and an unencrypted Godot 4 PCK v2.
Run from any directory with Python 3; no editor/import cache is required.
"""
from pathlib import Path
import argparse
import hashlib
import struct
import zlib

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "test_project"

def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff)

def pixel(x, y):
    if x in (0, 31) or y in (0, 31):
        return (255, 255, 255, 255)
    if 12 <= x < 20 and 12 <= y < 20:
        return (0, 0, 0, 0)
    return ((255, 0, 0, 255), (0, 255, 0, 255), (0, 0, 255, 255), (255, 255, 0, 128))[(y // 16) * 2 + x // 16]

def atlas_pixel(x, y):
    # Six 32x24 tiles in a 96x48 NPOT image, with an asymmetric orientation mark.
    tile = (y // 24) * 3 + x // 32
    x, y = x % 32, y % 24
    if (2 <= x < 5 and 2 <= y < 12) or (2 <= x < 12 and 2 <= y < 5):
        return (255, 255, 255, 255)
    if 14 <= x < 18 and 10 <= y < 14:
        return (0, 0, 0, 0)
    color = ((255, 0, 0), (0, 255, 0), (0, 0, 255),
             (255, 255, 0), (0, 255, 255), (255, 0, 255))[tile]
    return (*color, 128 if y >= 18 else 255)

def build(regions=False, primitives=False, camera=False):
    project = ROOT / "region_test_project" if regions else PROJECT
    if primitives: project = ROOT / "primitive_test_project"
    if camera: project = ROOT / "camera_test_project"
    width, height, sample = (96, 48, atlas_pixel) if regions else (32, 32, pixel)
    image_name = "atlas.png" if regions else "sprite.png"
    rows = b"".join(b"\0" + bytes(c for x in range(width) for c in sample(x, y)) for y in range(height))
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)) + chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b"")
    (project / image_name).write_bytes(png)
    files = [(name, (project / name).read_bytes()) for name in ("project.godot", "main.tscn", "main.gd", image_name)]
    files.extend((p.name, p.read_bytes()) for p in sorted(project.glob("*.gd")) if p.name != "main.gd")
    # This fixture has no class_name declarations; exported games still need this cache.
    files.append((".godot/global_script_class_cache.cfg", b"list=[]\n"))
    paths = []
    for name, _ in files:
        path = ("res://" + name).encode()
        paths.append(path + b"\0" * ((-len(path)) % 4))
    offset = 100 + sum(4 + len(path) + 36 for path in paths)
    directory, payload = bytearray(), bytearray()
    for path, (_, data) in zip(paths, files):
        directory += struct.pack("<I", len(path)) + path
        directory += struct.pack("<QQ", offset, len(data)) + hashlib.md5(data).digest() + struct.pack("<I", 0)
        payload += data
        offset += len(data)
    header = struct.pack("<4s5IQ", b"GDPC", 2, 4, 3, 0, 0, 0) + bytes(64) + struct.pack("<I", len(files))
    output = ROOT / "romfs" / "Game.pck"
    backup = ROOT / "Game-headless-original.pck"
    if not backup.exists():
        backup.write_bytes(output.read_bytes())
    output.write_bytes(header + directory + payload)
    print(f"Packed {len(files)} project files into {output} ({output.stat().st_size} bytes)")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--regions", action="store_true", help="Package Milestone A; keep the earlier fixture unchanged")
    group.add_argument("--primitives", action="store_true", help="Package Milestone B")
    group.add_argument("--camera", action="store_true", help="Package Camera2D fixture")
    args = parser.parse_args()
    build(args.regions, args.primitives, args.camera)
