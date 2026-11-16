# Sprite2D / Citro2D milestone

Historical single-sprite checkpoint. The user subsequently confirmed its visible
output. For recovered transforms/clipping/diagnostics and the newer nine-sprite
test build, see [DEVELOPMENT_PROGRESS.md](../../DEVELOPMENT_PROGRESS.md).
Do not copy a new build over the known-working binary named below.

Status: implemented and compiled; no visual emulator or hardware verification of this build.
Previous CPU blue-screen and diagnostic .3dsx binaries remain in bin/ unchanged.

## Actual graphics path

The packed Godot project loads a PNG using Image.load_png_from_buffer, assigns an
ImageTexture to a real Sprite2D node, and animates its position from GDScript.
Sprite2D emits a normal canvas texture-rectangle command. RendererCanvasCull supplies
the visible items, transforms and modulation. Canvas3DS consumes those commands;
TextureStorage3DS resolves the texture RID and uploads Godot image bytes to a
Citro3D texture. Citro2D submits the textured rectangle to the top-screen target.
The backend contains no fixture image, scene coordinates or hardcoded sprite.

The CPU framebuffer clear/SELECT rectangle loop is removed from this build so it
cannot overwrite GPU output or swap the same top framebuffer a second time.
Citro3D owns begin/end/presentation. The bottom console stays a CPU framebuffer,
and the existing START exit is retained. aptMainLoop also handles app lifecycle.

## Expected test output

- Native 400x240 blue top screen.
- A solid green 64x48 rectangle at (40,30), from Godot Node2D._draw().
- A 32x32 Sprite2D texture scaled 2x, moving horizontally around (200,120).
- Texture quadrants: red top-left, green top-right, blue bottom-left,
  half-transparent yellow bottom-right; white outline and transparent center.
- Bottom messages include project texture assignment, texture upload and the first
  Godot textured-rectangle submission. These messages prove submission, not GPU output.
- START exits; SELECT is not used in this project.

## Deliberately limited

One native top-screen viewport; untextured and textured RECT commands and command
transforms; translation and positive axis-aligned scale; nearest filtering;
RGBA8 uploads with power-of-two padding and source regions inside the image;
color/alpha modulation. GPU textures are cached and retired after frame sync.
The fixture needs only 4096 texture bytes on the GPU; C2D batching is limited to
256 objects rather than its 4096-object default.

No new 3D renderer, custom shaders, lighting, materials, rotation, skew, flipping,
custom clipping, polygons, tile repeats, canvas textures/proxies, mipmaps,
multiple/offscreen viewports or render-target sampling. These are not a general
2D game renderer yet. Unsupported commands/rectangle flags/transforms warn once
and skip; unsupported inherited dummy APIs remain no-ops. GPU allocation/draw
failures are diagnosed. Existing Godot engine modules were not stripped further.
This is not evidence of O3DS compatibility or acceptable full-game RAM usage.

## Build

In PowerShell at the repository root (adjust SDK path if needed):

```powershell
$env:DEVKITPRO = "C:/devkitPro"
$env:DEVKITARM = "C:/devkitPro/devkitARM"
$env:Path = "C:\devkitPro\devkitARM\bin;C:\devkitPro\tools\bin;" + $env:Path
python platform/3ds/pack_test_project.py
python -m SCons platform=3ds target=template_release devkitpro=C:/devkitPro -j8
Copy-Item bin/godot.3ds.template_release.arm32.elf.3dsx bin/godot-sprite2d-citro2d.3dsx
```

The packer uses the checked-in Godot project, regenerates its tiny PNG and writes
Godot PCK v2. No desktop import cache, editor or proprietary SDK is required.
The original headless project's pack is retained as Game-headless-original.pck
outside romfs so it is not bundled unnecessarily. Packaging subprocess errors now
fail the SCons action instead of being ignored.

## Validation completed

1. Rectangle compositor/viewport checkpoint compiled after a single const-owner fix.
2. Texture-cache/canvas checkpoint compiled.
3. Final backend and test-project packaging compiled/linked successfully.
4. Upload layout reference compared byte-for-byte with installed official tex3ds
   RGBA8 output: all 4096 pixel bytes match. This caught and corrected a row flip.
5. All four PCK entries were independently checked for offsets, content and MD5.
6. Final 3DSX header/SMDH, embedded Sprite2D PCK and backend marker were checked.

Still to test in Azahar and on hardware: actual sprite visibility/orientation,
alpha blending, motion, viewport presentation, console persistence, START exit,
suspend/resume and memory stability. The computer-use tool was unavailable during
this run; no visual success claim is made.

## Changed files for this milestone

- platform/3ds/servers/rendering/rasterizer_3ds.h (new)
- platform/3ds/servers/rendering/rasterizer_3ds.cpp (new)
- platform/3ds/servers/display/display_server_3ds.h
- platform/3ds/os_3ds.cpp
- platform/3ds/detect.py
- platform/3ds/SCsub
- main/main.cpp (startup comment updated; existing 3DS selection logic retained)
- platform/3ds/pack_test_project.py (new)
- platform/3ds/test_project/project.godot (new)
- platform/3ds/test_project/main.tscn (new)
- platform/3ds/test_project/main.gd (new)
- platform/3ds/test_project/sprite.png (new)
- platform/3ds/romfs/Game.pck (replaced by test project)
- platform/3ds/Game-headless-original.pck (original backup)
- platform/3ds/godot.smdh (generated test title)
- platform/3ds/SPRITE2D_MILESTONE.md (this file)

Build-generated object files, libraries and binaries are not source changes.
