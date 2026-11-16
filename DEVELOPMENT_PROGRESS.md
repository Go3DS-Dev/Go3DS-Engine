# Current checkpoint: Milestones B and C delivered (2026-11-16)

## Builds preserved for manual visual testing

B — basic primitives:
`C:\Users\User\Documents\Codex\2026-11-15\new-chat\godot-4-3ds\bin\godot-primitives-milestone-b-20261116.3dsx`
26,423,844 bytes; SHA256 `86182b7aee782794f8b36daf074cd20d64211a108f2da1c13db12ec77d0dfcd2`.

C — Camera2D fixture using the same renderer:
`C:\Users\User\Documents\Codex\2026-11-15\new-chat\godot-4-3ds\bin\godot-camera-milestone-c-20261116.3dsx`
26,422,972 bytes; SHA256 `e8f479d3eed4b50a906f76134b05057260ab5603923b3d71687fe3d78811300f`.

All earlier named binaries, including verified Milestone A, retain their pre-session
hashes. No Git metadata in this extracted tree. No AGENTS.md found in the workspace
parent or repository. Snapshot and hash manifest are in ../work/primitives-recovery-20261116.zip
and ../work/primitives-preserved-builds.json. No work was reset.

## B implementation

- Filled draw_rect already worked and continues through the same RECT path.
- Untextured PRIMITIVE commands now render 2-point hairlines, 3-point triangles,
  and 4-point quads. Godot expands positive-width draw_line to a quad; we split it
  into two triangles. Width therefore follows the complete canvas affine transform.
- Negative-width lines are transformed to screen coordinates before C2D_DrawLine
  with width 1, so camera/node scale does not enlarge hairlines. These require a
  batch flush/model reset, not an extra GPU frame synchronization.
- Polygon request/free now retain Godot's COW point/color/index arrays under IDs.
  No per-frame triangulation or allocations in the draw loop. Geometry is released
  when Godot frees its canvas command; animation changes node transforms only.
- Untextured triangle lists/strips and line lists/strips supported. Godot supplies
  outlined rectangle geometry, 64-segment filled circle geometry, and triangulates
  draw_colored_polygon (including the simple concave arrow fixture).
- Primitives and polygons use the same clip rectangle, transforms, item/canvas
  modulation, per-vertex alpha, and ordered command loop as sprites. No texture
  upload is needed for shapes. Existing Sprite2D/atlas upload and UV code untouched.
- Input indices/point counts/finite coordinates are checked. Textured/skinned
  polygons, textured primitives and unsupported topologies warn/skip. Polygon
  requests are bounded to 4096 points / 12288 indices. Capacity exhaustion warns;
  there is no unbounded buffer growth. Citro2D's existing 256-object capacity remains.

## C investigation and fixture

Camera2D::_update_scroll writes viewport canvas transform. Its get_camera_transform
already computes inverse translation/zoom/rotation. RendererViewport passes the
combined transform to RendererCanvasCull; item->final_transform includes it.
Canvas3DS already consumes final_transform. No extra camera transform was added
(which would have applied it twice). C required NO additional renderer changes.
New fixture follows a moving Sprite2D with a child Camera2D, changes uniform zoom
0.75–1.25 and rotation +/-0.3 radians, draws world landmarks/grid, and uses a fixed
CanvasLayer HUD. Both sprites share one 32x32 texture.

## Build and validation results

- build-primitives-baseline.log: unchanged baseline success.
- build-primitives-lines.log: primitive/line compilation and linking success.
- build-primitives-polygons.log: polygon storage/topology compilation success.
- build-primitives-final.log and build-camera-final.log: final packaging success.
- Desktop Godot 4.7.2: B scene ran 300 fixed-60-FPS frames without script/cache
  errors (build-primitives-desktop.log). This is not a measured performance test.
- Sequential validate_canvas_fixture.gd runs passed independently for B and C.
  B checks texture, shape/sprite ordering, clipping setup and moving/scaled nodes.
  C checks 12 samples: followed sprite maps to (200,120), correct zoom basis length,
  inverse camera rotation, shared texture, fixed HUD canvas transform.
- Desktop runs report a separate Windows certificate-store error. No Windows
  protections/settings were changed. Port is Godot 4.3 RC; desktop is 4.7.2, so
  desktop tests do not substitute for ARM runtime or Citro2D visual validation.
- validate_embedded_project.py passed for BOTH named builds: PCK entries/checksums,
  exact project source match, global script-class cache, SMDH/3DSX, diagnostics,
  and all earlier named binaries unchanged. It reads each embedded PCK rather than
  assuming current romfs/Game.pck matches that build.
- Final Azahar attempt: selected its current window; accessibility unavailable and
  screenshot FrameArrived timed out. Neither B nor C visually verified by automation.

## What to test in Azahar, then hardware

Test B first:
- Blue 400x240 top screen. Upper-left horizontal/vertical/diagonal lines with
  clearly different widths; upper row green filled and white/yellow outlined boxes.
- Middle-left translucent magenta circle and orange concave arrow.
- Middle rotating/scaled rectangle with outline and diagonal yellow line.
- Bottom-left teal shape behind a four-color Sprite2D, with a translucent yellow
  bar and white hairline IN FRONT of the sprite despite earlier scene-tree order.
- Right white frame clips a moving orange circle and diagonal white line; no
  geometry outside its interior. Check outlines, alpha and triangle seams carefully.

Then C:
- Player remains centered between stationary white HUD ticks at (200,120).
- Grid, colored landmarks and other sprite scroll around it, smoothly zoom and
  rotate. Green HUD marker at upper-left must not move/zoom/rotate.
- World geometry should not disappear unexpectedly as camera moves.

Both:
- Bottom console and START exit retained. No unsupported/capacity warnings expected
  for the fixture. Texture statistics should settle to `GPU tex 1/4 KiB up/free 1/0`,
  retired 0, and remain stable. Watch FPS/heap/linear memory for several minutes.
- New 2D geometry increases draw work; no 60 FPS/O3DS performance claim for B/C.
  Two circles in B use 128 triangles total; C uses one 64-triangle circle.
  No full leak audit or repeated redraw/free stress test completed. Stable fixture
  counters alone cannot prove absence of leaks. Godot release allocation tracking
  remains unavailable; newlib/linear reports are not total system RAM.
- Hardware startup, visual correctness, suspend/resume and stability still require
  real-console testing. No O3DS compatibility claim.

## Remaining limitations

No textured/skinned polygons or primitives, mesh/multimesh rendering, point primitives,
custom materials/shaders, lighting, offscreen/multiple viewports, nine-patch or tiling.
Nearest RGBA8 sprite textures only. Inherited unsupported dummy APIs remain no-ops.
AA feather geometry can pass through colored triangles, but antialiased variants
are not visually verified and are not exercised in these fixtures. Hairline endpoint
rasterization may differ from desktop GPU lines; verify appearance. Polygon self-
intersections/holes rely on Godot's triangulator and are not supported/tested here.
No camera smoothing/limits/drag or multiple-camera/viewport validation yet.

## Changed files in B/C session

- platform/3ds/servers/rendering/rasterizer_3ds.cpp
- platform/3ds/servers/rendering/rasterizer_3ds.h
- platform/3ds/pack_test_project.py (mutually exclusive --regions/--primitives/--camera;
  packs each fixture's additional .gd files; original default unchanged)
- platform/3ds/primitive_test_project/{project.godot,main.tscn,main.gd,shapes.gd,sprite.png}
- platform/3ds/camera_test_project/{project.godot,main.tscn,main.gd,hud.gd,sprite.png}
- platform/3ds/tests/validate_canvas_fixture.gd
- platform/3ds/tests/validate_embedded_project.py
- platform/3ds/romfs/Game.pck (currently C fixture), DEVELOPMENT_PROGRESS.md
- Named B/C binaries and generic build outputs/logs/compilation database.
Existing test_project and region_test_project were not edited.

## Reproduce / exact next step

Use the SDK/SCons command in SPRITE2D_MILESTONE.md, preceded by:
`python platform/3ds/pack_test_project.py --primitives` for B, or `--camera` for C.
Copy the generic .3dsx to a NEW filename; do not overwrite these checkpoints.
Package validation: `python platform/3ds/tests/validate_embedded_project.py
<binary filename> <primitive_test_project or camera_test_project>`.
Fixture validation: desktop Godot `--headless --main-pack <Game.pck> --script
<absolute platform/3ds/tests/validate_canvas_fixture.gd>`. Wait for the process to
exit before replacing the PCK (GUI executables may launch asynchronously in PowerShell).

STOP at these checkpoints. Exact next step: user's Azahar B test first, then C;
collect screenshots, console warnings and FPS/memory statistics. Fix the first
observed defect before expanding functionality. Following acceptance, investigate
repeated redraw/free stress and real O3DS performance before more complex features.

---

Earlier incremental notes follow (historical, superseded by the status above).

# B and C compiled checkpoints — latest status

Milestone B: bin/godot-primitives-milestone-b-20261116.3dsx
26,423,844 bytes; SHA256 86182b7aee782794f8b36daf074cd20d64211a108f2da1c13db12ec77d0dfcd2.
Milestone C: bin/godot-camera-milestone-c-20261116.3dsx
Both under C:\Users\User\Documents\Codex\2026-11-15\new-chat\godot-4-3ds.
B and C compilation/packaging passed. Separate sequential desktop fixture tests
passed (build-primitives-validation.log and build-camera-validation.log), including
12 follow/zoom/rotation/HUD coordinate samples for C. Earlier concurrent GUI test
launch results were not used. Windows certificate-store warning remains unrelated.
Only B required renderer changes; C is a fixture exercising existing Godot transforms.
Neither new build is visually verified on emulator or hardware yet.
Next: finish C package check and final documentation; test B in Azahar before C.
No further engine features planned in this session.

# Camera2D inspection (B checkpoint preserved)

B package validator passed; SHA256 86182b7aee782794f8b36daf074cd20d64211a108f2da1c13db12ec77d0dfcd2.
Camera2D::_update_scroll writes viewport canvas transform; get_camera_transform
already computes zoom/rotation/translation inverse. RendererViewport passes this
to RendererCanvasCull, which composes item transforms and supplies final_transform.
Canvas3DS already consumes that transform. NO backend camera changes needed.
Added focused camera_test_project with following Camera2D, animated zoom/rotation,
world grid/landmarks, shared Sprite2D texture, fixed CanvasLayer HUD. Next: validate
world-to-screen coordinates on desktop, package --camera and build separately.

# Milestone B packaged checkpoint (latest)

B compiled and packaged as:
C:\Users\User\Documents\Codex\2026-11-15\new-chat\godot-4-3ds\bin\godot-primitives-milestone-b-20261116.3dsx
Do not overwrite. build-primitives-final.log passed. Desktop Godot 4.7.2 ran
packaged scene 300 fixed-60-FPS frames, exit 0, no script/cache errors; separate
Windows certificate-store error remains. New GPU output NOT visually verified.
Fixture shows width-varied lines, filled/thick/hairline outlined rectangles,
rotating/scaled shapes, two filled circles, concave arrow, clipped shapes and
shape/sprite/shape interleaving. One shared 32x32 sprite texture (4 KiB GPU).
Modified: rasterizer_3ds.cpp/.h, pack_test_project.py; added primitive_test_project
(project.godot, main.gd, main.tscn, shapes.gd, sprite.png); generated romfs/Game.pck.
Geometry stays in Godot's normal commands and polygon lifetime. Filled rectangles
reuse previous implementation; no new circle tessellator or polygon triangulator.
Next: independent package checks, then Camera2D source inspection and focused
fixture only if the existing final-transform path is sufficient. Preserve B.

---

# B polygon checkpoint compiled

Added Canvas3DS polygon cache/request/free in rasterizer_3ds.h/.cpp; build-primitives-polygons.log passed.
Supports untextured triangle lists/strips and hairline lists/strips; Godot supplies
outline and 64-segment circle geometry and triangulates simple filled polygons.
Retains COW arrays across frames, releases on free_polygon. Textured/skinned
polygons rejected. No visual verification yet. Next: package primitive fixture, validate,
compile and preserve named B binary before Camera2D investigation.

# Milestone B in progress — latest status

User confirms A works in Azahar at ~60 FPS with one upload. No hardware confirmation.
No AGENTS.md found in workspace parent/repository; no Git metadata.
Source snapshot: ../work/primitives-recovery-20261116.zip; build hashes preserved.
Inspection: filled rectangles already work. Lines use PRIMITIVE (2 or 4 points);
outlined rectangles use POLYGON line/triangle strips. Circles are already tessellated
by Godot (64 triangles); polygons already triangulated by Godot. Dummy polygon storage
currently drops them. Baseline and untextured 2/3/4-point primitive checkpoint compiled successfully
(build-primitives-baseline.log, build-primitives-lines.log). Changed rasterizer_3ds.cpp.
Positive-width lines use Godot quads; negative-width lines are one screen pixel.
Next:
then retain Godot polygon data and support its topology. Do not change Sprite2D path.

---

# 2D checkpoint recovery â€” 2026-11-16

## Milestone A complete and awaiting Azahar test (latest status)

User verified the expanded checkpoint in Azahar: transforms, rotation, flips,
negative scaling, alpha, clipping and order appear correct at about 60 FPS.
No hardware/O3DS verification. Existing named binaries are preserved.

Baseline build passed (build-regions-baseline.log). Source snapshot before edits:
../work/regions-recovery-20261116.zip. Hash manifest: ../work/regions-preserved-builds.json.
This folder still has no Git metadata.

Region UV conversion already existed in get_image. Added finite-region validation,
empty-region early return and rounded-up Tex3DS size metadata for fractional regions.
No changes to transforms, flips, scissor or frame lifecycle. Backend compiled
successfully (build-regions-renderer.log). New region_test_project uses a 96x48
atlas shared by 16 Sprite2D nodes plus a CanvasItem.draw_texture_rect_region command.
Old test_project is preserved; pack_test_project.py --regions selects the new fixture.

Final SCons packaging passed (build-regions-final.log), no compiler/linker errors.
New binary:
`C:\Users\User\Documents\Codex\2026-11-15\new-chat\godot-4-3ds\bin\godot-regions-milestone-a-20261116.3dsx`
26,418,592 bytes, SHA256:
`fdc89956a3948e7a2ffbee9ab3448d430812a4d6a892eaa008949a68488430fa`.

### Scope and implementation

Supported canvas commands remain RECT (filled or textured), TRANSFORM and CLIP_IGNORE.
RECT supports REGION, FLIP_H, FLIP_V and accepts CLIP_UV. Sprite2D already generates
REGION commands through texture->draw_rect_region; no scene/rendering bypass added.
Source UVs use padded GPU dimensions and positive, in-image source extents.
Region animation changes only the descriptor, not the cached texture RID/upload.
This milestone validates/hardens an existing region path rather than inventing a new backend.

Important missing features remain line/primitive/polygon commands, circles, nine-patch,
repeat/tiling, transpose, custom materials, fonts and multiple/offscreen viewports.
Only nearest filtering is implemented. Filter-clip has no additional shader clamp;
the integer pixel atlas tests use nearest sampling. Fractional regions retain float
UVs with rounded descriptor metadata, but require separate visual verification.
Regions outside the image warn and skip; wrapping/repeating is not implemented.
Camera2D has not been investigated in this milestone; do not duplicate transforms
without inspecting the engine's existing canvas path when Milestone C is authorized.

### Validation completed

- Unchanged baseline build, renderer safeguard compilation, final packaging: passed.
- Desktop Godot 4.7.2 script check: passed (build-regions-script.log).
- validate_regions.gd against the embedded PCK: passed; cache/native size, 16 shared
  Sprite2D textures, six regions and colors, smaller crop, alpha markers, transform/
  flip/clip/order setup, and twelve animated region selections checked.
- Headless packaged scene ran 420 fixed-60-FPS frames, exit 0; no script errors or
  global script-class cache errors (build-regions-runtime.log). Desktop Windows
  reported the previously observed certificate-store error; no security changes made.
- Official installed tex3ds RGBA8 raw conversion matches all 32,768 padded atlas
  bytes (zero differences), including padding and Morton tile layout.
- validate_region_package.py: passed checksums, all five PCK entries, source match,
  cache, 3DSX/SMDH/RomFS, embedded project and backend diagnostic markers.
- All five previous named binaries retain their pre-edit SHA256 hashes. Prior
  test_project project/scene/script/PNG contents are unchanged.

Desktop fixture validation does NOT validate ARM execution or Citro2D output.
The previous Azahar UI/capture attempt was unavailable; no visual test of this new
binary was performed in this session. No hardware compatibility claim.

### Visual test instructions

- Top row: six separate 48x36 displayed tiles, red/green/blue/yellow/cyan/magenta.
  Each has a white L at upper-left, transparent center and translucent bottom band.
  There should be no neighboring colors, stretched atlas or black padding sampled.
- Middle-left cyan region rotates; moving sprite changes through all six colors
  once per second while moving horizontally. Its size stays 48x36.
- Small green crop near (240,90) uses an internal 16x12 source rectangle, not a whole tile.
- Bottom-left four cyan tiles: H flip, V flip, negative X, negative Y. First/third
  and second/fourth should match. Rotations/reflections must not disappear.
- Magenta direct CanvasItem region is drawn at (175,145), 64x36; green filled
  rectangle remains at (15,145). No line/circle/polygon functionality was added.
- Upper-right magenta tinted translucent sprite overlaps red despite earlier tree
  order. Lower-right white frame clips a moving/rotating yellow region.
- Blue background, native 400x240 top, bottom console and START exit remain.
- Diagnostics should settle at `GPU tex 1/32 KiB up/free 1/0`, retired 0. Region
  changes must not increase uploads. Watch FPS, heap and linear memory for minutes.
  32 KiB vs prior 4 KiB is the larger padded atlas, not a leak. No new per-region
  allocations, uploads, synchronization or draw calls were added to the renderer.
  The scene has 16 sprites plus a direct textured rectangle, so it submits more
  geometry than the prior nine-sprite demo. No new performance measurements yet.

### Files modified/added for A

- platform/3ds/servers/rendering/rasterizer_3ds.cpp (small region guards/metadata)
- platform/3ds/pack_test_project.py (--regions switch and deterministic atlas)
- platform/3ds/region_test_project/project.godot (new)
- platform/3ds/region_test_project/main.tscn (new)
- platform/3ds/region_test_project/main.gd (new)
- platform/3ds/region_test_project/atlas.png (generated, new)
- platform/3ds/tests/validate_regions.gd (new)
- platform/3ds/tests/validate_region_package.py (new)
- platform/3ds/romfs/Game.pck (generated)
- DEVELOPMENT_PROGRESS.md (this report)
- New named binary, generic build outputs/logs and compilation database.

Build using the existing SDK/SCons command below, but first run
`python platform/3ds/pack_test_project.py --regions`. Without --regions, the packer
still packages the preserved earlier demo. Validate A with
`python platform/3ds/tests/validate_region_package.py`; desktop scene assertions use
Godot `--headless --main-pack platform/3ds/romfs/Game.pck --script <absolute validate_regions.gd>`.

Exact next step: STOP and await the user's Azahar test of this named binary.
If regions fail, fix the first visual defect. Only after the user accepts A, inspect
Godot line/primitive commands for Milestone B. Do not start B or C in this session.

---

The following sections record the earlier checkpoint and its then-current status.

## Recovered before editing

This directory is an extracted source tree, not a Git checkout (git status fails).
No work was reset. Recovery snapshot: ../work/recovery-2d-20261116.zip.
SPRITE2D_MILESTONE.md describes the earlier single-sprite milestone, not the newer work.

- User confirmed the earlier Sprite2D binary displays a moving textured sprite.
- Cache packaging, full canvas matrices, negative scale, H/V flip flags, scissor
  clipping, existing item order, and lightweight diagnostics are implemented.
- build-cache-fix.log, build-transforms.log, build-clipping.log, build-stats.log
  all end in successful builds. Latest recovered build: generic ELF.3dsx,
  26,413,876 bytes; diagnostics build completed at 08:40 host time.
- main.gd/main.tscn are still the original single-sprite demo. Expanded demo unfinished.
- New rendering features have NOT been visually verified in emulator or hardware.
- Existing bottom console and START exit remain in OS_3DS::run.

## Completed in recovery session

- Expanded main.tscn/main.gd to nine actual Sprite2D nodes sharing one ImageTexture.
  No fixture coordinates or animation were added to the backend.
- Added parent/child rotation and nonuniform scaling, flip_h/flip_v comparisons
  with negative X/Y scale, overlapping alpha/color modulation, and Control clipping.
- Reused previously compiled diagnostics: FPS, Citro3D CPU/GPU times, command-buffer
  peak, free linear memory, newlib heap usage, GPU texture bytes/uploads/frees.
- Repacked and successfully rebuilt with SCons (build-2d-checkpoint.log).
  Backend objects were already current; no renderer C++ modifications in this recovery.
- Added repeatable fixture and binary validators. Updated this report and historical note.

## Deliverable

`C:\Users\User\Documents\Codex\2026-11-15\new-chat\godot-4-3ds\bin\godot-2d-checkpoint-20261116.3dsx`

26,416,112 bytes. SHA256:
`96d1af9a03c66011a935de97c3990e9b83e3ed92407fc579a5cff1cc542a89b5`

Preserved working binary: bin/godot-sprite2d-citro2d.3dsx, SHA256:
`f557a4813ab7d6d45e7211f048fa60a7dc0e9a4205600240de7093714b698ab4`.
Original framebuffer/baseline builds are also preserved.

## Tests and limits of evidence

1. Desktop Godot 4.7.2 script check: exit 0 (build-demo-script.log).
2. Embedded PCK ran headless for 180 fixed-60-FPS frames: exit 0. No script-cache
   or GDScript errors. Windows reported a separate root certificate-store error;
   no security settings were changed (build-demo-package.log).
3. validate_fixture.gd loaded the embedded cache and scene, checked native 400x240,
   all nine sprites sharing the same texture RID, transforms, flags, alpha pixels,
   clip size, Z ordering versus tree ordering, and animation: FIXTURE PASS.
   Same Windows certificate-store warning (build-fixture-validation.log).
4. validate_checkpoint.py: all five PCK entries, bounds, checksums, source identity,
   valid cache, 3DSX/SMDH/RomFS and embedded project, diagnostics marker: PACKAGE PASS.
   Also verified the old working binary's SHA256 remains unchanged.
5. Azahar UI attempt: no accessibility state; screenshot timed out. The new build
   was NOT visually verified or confirmed launched by automation.

Desktop tests use a newer Godot than the 4.3 RC port: they validate the fixture,
not its ARM execution or Citro2D output. The cache cause is fixed in the package
and desktop loading verified; confirm absence of the error on the 3DS runtime.

## What to observe manually

- Native blue 400x240 background and original green rectangle near upper-left.
- Rotating sprite around (145,45); moving sprite around (180,105).
- Upper-right row, left to right: flip H, flip V, negative X, negative Y.
  H and negative X should match; V and negative Y should match.
  Unflipped texture is red/green above blue/half-transparent yellow, white border,
  transparent center. Mirrored sprites must not vanish or shift their centers.
- Lower-left overlap: tinted translucent sprite at (90,180) is in front of the
  sprite at (65,155), despite appearing earlier in the scene tree. Background and
  back sprite should show through transparent regions.
- Lower-right white frame encloses a 96x80 dark clipping area at (260,130).
  Large rotating/moving sprite must stay inside it, including when crossing edges;
  it must not erase the white border or clip unrelated sprites.
- Bottom console remains live, printing diagnostics roughly every five seconds.
  Shared texture should settle at `GPU tex 1/4 KiB up/free 1/0`; animation must
  not increase upload count. Retired count should be zero in this static-texture demo.
- Run for several minutes: look for steady FPS, stable heap/linear memory after
  startup, and no increasing upload count. These observations do not prove absence
  of all leaks. This scene does not exercise repeated texture replacement/free.
- START exits. Check suspend/resume separately on hardware.

## Remaining limitations / exact next step

STOP here and visually test the delivered binary. Record emulator/version or
console model, FPS, heap/linear memory and any incorrect sprite orientation or
clipping. Fix the first observed defect before adding new engine features.

Rotation, reflections, clipping, alpha/order and new diagnostics are compiled but
remain visually unverified on 3DS. No O3DS compatibility/performance claim.
GPU timing is Citro3D work, not whole-engine CPU utilization. Newlib heap/linear
statistics are not total system RAM. Emulator application-region accounting may
still be invalid and is explicitly reported as unavailable. Release Godot allocation
tracking reports zero. No full leak/stress audit completed.

Backend remains single native top viewport, nearest RGBA8 textures, rectangle canvas
commands; unsupported commands warn/skip or retain dummy implementations. No fonts,
3D, custom shaders, offscreen viewports, lighting or advanced effects added.

## Files changed in this recovery

- platform/3ds/test_project/main.gd
- platform/3ds/test_project/main.tscn
- platform/3ds/romfs/Game.pck (generated)
- platform/3ds/tests/validate_fixture.gd (new; not packaged)
- platform/3ds/tests/validate_checkpoint.py (new)
- platform/3ds/SPRITE2D_MILESTONE.md (historical-note link)
- DEVELOPMENT_PROGRESS.md (new)
- bin/godot-2d-checkpoint-20261116.3dsx (new deliverable)
- Generic build output, build logs and compilation database were regenerated.
  PNG and SMDH generation was rerun; no intended content change to those fixtures.

## Reproduction

Use the SDK environment/build command documented in SPRITE2D_MILESTONE.md.
Run `python platform/3ds/pack_test_project.py` before SCons; copy its generic .3dsx
output to a NEW versioned filename (never overwrite the known-working binary).
Run `python platform/3ds/tests/validate_checkpoint.py` for this named checkpoint.
Desktop fixture test: Godot `--headless --main-pack platform/3ds/romfs/Game.pck
--script <absolute path to platform/3ds/tests/validate_fixture.gd>`.
