# 🎮 Go3DS Engine

**Godot 4 for Nintendo 3DS**

*Make games in Godot. Play them on your Nintendo 3DS.*

Go3DS Engine is an experimental, open-source port of the Godot 4 game engine for the Nintendo 3DS family of handheld consoles.

Built upon the original [Godot 4 Nintendo 3DS port by SeleDreams](https://github.com/SeleDreams/godot-4-3ds), Go3DS aims to make it possible to develop homebrew games using familiar Godot features such as scenes, nodes, GDScript, sprites, and 2D physics.

The project currently focuses on **2D game development**, with the original Nintendo 3DS as its primary performance target.

Go3DS is an unofficial community project and is not affiliated with Nintendo or the Godot Foundation.

---

## 🚧 Development Status

**Go3DS is currently experimental and under active development.**

The original port could execute GDScript but lacked a functional renderer. Go3DS development has since introduced an experimental 2D rendering system using Citro2D and Citro3D.

Basic rendering tests have successfully run in the Azahar Nintendo 3DS emulator, including tests with multiple sprites, texture regions, transformations, and camera movement.

The engine is not yet ready for general-purpose game development. Features may be incomplete, unstable, or subject to change.

There are currently no guarantees of compatibility with existing Godot projects.

---

## 🤖 100% Vibe-Coded

**Yes. Go3DS is fully vibe-coded.**

The new development of Go3DS Engine is driven through AI-assisted programming, primarily using **OpenAI Codex**.

Rather than manually implementing each feature, development is directed through prompts, experimentation, build testing, debugging, and iterative feedback.

This includes work on the renderer, Nintendo 3DS platform integration, compatibility improvements, and additional engine functionality.

### What does this mean?

- New Go3DS development is primarily generated and modified by AI.
- The project maintainer directs development goals, features, and priorities.
- Implementations are evaluated through compilation, testing, and debugging.
- AI-generated code may contain bugs, inefficient implementations, or unexpected behavior.
- Community code reviews, bug fixes, and improvements are welcome.

**Important:** Go3DS builds upon existing human-developed open-source software, including Godot Engine and SeleDreams' original Nintendo 3DS port. Those contributions are not claimed as AI-generated work.

The project is transparent about its development process and aims to improve code quality, stability, and compatibility over time.

**100% vibe-coded. 100% open source. Made for the Nintendo 3DS.**

---

## ✨ Features

Go3DS extends the existing Godot runtime with Nintendo 3DS-specific functionality.

### 🎨 2D Rendering

The experimental renderer uses Nintendo 3DS graphics libraries, including Citro2D and Citro3D.

Features implemented or demonstrated in development builds include:

- Sprite2D rendering
- Texture loading and GPU texture caching
- Texture atlas regions
- Dynamic texture region switching
- Sprite rotation and scaling
- Horizontal and vertical flipping
- Transparency and alpha blending
- Canvas clipping
- Basic 2D drawing primitives
- Camera2D transformations
- CanvasLayer rendering
- Draw ordering and canvas transforms

Some features may have limitations compared with standard Godot 4 rendering.

### 📜 GDScript

Go3DS uses Godot's existing GDScript runtime.

The goal is to support normal Godot scripting workflows without requiring a separate programming language or custom game engine API.

### 🧩 Godot Scenes and Resources

Go3DS aims to preserve compatibility with standard Godot project formats:

- `.tscn` — Godot scenes
- `.tres` — Godot resources
- `.gd` — GDScript files
- `.pck` — Packaged game resources
- `project.godot` — Project configuration

Support for individual nodes and engine features depends on the current implementation.

### 🛠️ Debugging and Diagnostics

Development builds include diagnostic functionality such as:

- Runtime console output
- Frame-rate monitoring
- Memory statistics
- GPU texture upload statistics
- Renderer testing scenes

An experimental texture-atlas test using 16 Sprite2D nodes demonstrated approximately 60 FPS in Azahar, with a single GPU texture upload reported.

This is a specific test result, not a general performance guarantee.

---

## 🗺️ Development Roadmap

The main objective is to make Go3DS capable of running complete 2D games on Nintendo 3DS hardware.

| Feature | Status |
|---|---|
| Godot runtime startup | ✅ Working |
| GDScript execution | ✅ Working |
| Basic Sprite2D rendering | ✅ Experimental |
| Texture atlas regions | ✅ Experimental |
| Sprite transformations | ✅ Experimental |
| Camera2D support | ✅ Experimental |
| Basic 2D drawing primitives | 🧪 Experimental |
| CanvasLayer support | 🧪 Experimental |
| AnimatedSprite2D | 🚧 In development |
| Native Nintendo 3DS controls | 🚧 In development |
| 2D physics and collisions | 📋 Planned / validation pending |
| TileMapLayer | 📋 Planned |
| Scene switching | 📋 Planned / validation pending |
| Playable 2D platformer demo | 📋 Planned |
| Dual-screen rendering | 📋 Planned |
| Touchscreen support | 📋 Planned |
| Audio playback | 📋 Planned |
| Godot editor export integration | 📋 Long-term goal |
| 3D rendering | 📋 Not a current priority |

Statuses reflect the development milestones discussed so far and should be updated as implementation and testing progress.

### 🎯 Current Priorities

**Milestone 1 — Playable 2D Game**

Develop a basic platformer demonstrating sprites, animation, input, physics, collisions, and scene transitions.

**Milestone 2 — Nintendo 3DS Hardware Integration**

Improve compatibility with native controls, dual-screen rendering, and touchscreen input.

**Milestone 3 — Stability and Performance**

Optimize rendering, memory usage, resource management, and runtime performance on original Nintendo 3DS hardware.

**Milestone 4 — Developer Experience**

Simplify project packaging, documentation, build configuration, and eventually integration with the Godot editor's export workflow.

---

## 🎮 How It Works

Go3DS packages the Godot runtime into a Nintendo 3DS executable using the open-source devkitPro toolchain.

The resulting application is built as a `.3dsx` file.

### Project Packaging

The inherited port bundles the Godot project in the ROM filesystem located at:

`platform/3ds/romfs`

The runtime starts with arguments equivalent to:

```bash
--main-pack romfs:/Game.pck --display-driver 3ds --audio-driver Dummy
```

The packaged Godot project is loaded from:

```text
romfs:/Game.pck
```

To change which project is launched, replace the bundled `Game.pck` with a compatible Godot project package and rebuild the application.

The `Dummy` audio driver is currently used by this inherited startup configuration, so normal audio playback should not be assumed to work.

The long-term goal is to make this process much easier through improved packaging and export tooling.

---

## 🔧 Building Go3DS

Go3DS currently uses a development build process based on the original port.

### Requirements

You will need:

- Python and SCons
- devkitPro
- devkitARM toolchain
- libctru
- 3ds-freetype
- 3ds-bzip2
- 3ds-libpng
- 3ds-zlib

The renderer also uses Citro2D and Citro3D.

Additional development dependencies may be necessary depending on the current source revision.

### Installing devkitPro

Follow the official installation instructions:

https://devkitpro.org/wiki/Getting_Started

Make sure the devkitPro tools directory is available in your system's PATH.

The build system requires tools such as:

```text
3dsxtool
picasso
```

You can check whether `3dsxtool` is available by running:

```bash
3dsxtool
```

If the command cannot be found, check your devkitPro installation and PATH configuration.

### Compiling

From the repository root, the original port uses:

```bash
scons platform=3ds target=template_release devkitpro=/path/to/devkitpro
```

Replace `/path/to/devkitpro` with the path to your devkitPro installation.

**Note:** This is the inherited build command. Updated Go3DS build scripts or environment configurations may require additional arguments. Consult the repository's development documentation for the latest verified procedure.

### Running

Once compiled, the `.3dsx` executable can be tested using a compatible Nintendo 3DS emulator, such as Azahar, or on supported homebrew-enabled Nintendo 3DS hardware.

Emulator behavior and performance may differ from real hardware.

---

## 🕹️ Developing Games with Go3DS

The intended workflow is:

1. Create a lightweight 2D game using a compatible Godot 4 editor.
2. Use supported Godot nodes, scenes, resources, and GDScript functionality.
3. Package the project into a `Game.pck` file.
4. Include the package in the Go3DS ROM filesystem.
5. Build the Nintendo 3DS executable.
6. Test the game in an emulator or on compatible hardware.

Not all Godot nodes or features are supported yet.

Go3DS is currently a developing runtime port, not a complete replacement for the standard Godot editor.

### Performance Considerations

The Nintendo 3DS has significantly more limited processing power and memory than modern desktop systems.

Developers should favor:

- Lightweight 2D scenes
- Small and reusable textures
- Texture atlases
- Reasonable sprite counts
- Efficient scripts
- Minimal unnecessary allocations
- Careful memory management

Actual performance will depend on the game, hardware model, and engine implementation.

---

## ⚠️ Known Limitations

Go3DS is still experimental.

Current limitations include:

- Incomplete Godot 4 feature compatibility.
- An experimental 2D renderer.
- Incomplete or unverified support for certain CanvasItem operations.
- Limited hardware-specific integration.
- No established general-purpose export workflow.
- Audio support not yet established.
- 3D rendering is not currently a development priority.
- Potential differences between emulator and real hardware behavior.

Bugs, crashes, graphical glitches, and performance issues should be expected.

---

## 📜 License and Legality

Go3DS is built upon the open-source [Godot Engine](https://godotengine.org/), which is distributed under the MIT license.

The original Godot copyright notices, license files, and applicable third-party licensing requirements must be preserved.

Go3DS uses open-source Nintendo 3DS homebrew development tools rather than Nintendo's proprietary SDK.

No proprietary Nintendo SDK is required by the documented development workflow.

Using an open-source toolchain does not automatically resolve every possible legal question. Developers remain responsible for the legality and licensing of their own projects, assets, dependencies, and distribution methods.

**Go3DS is not affiliated with, sponsored by, or endorsed by Nintendo or the Godot Foundation.**

Nintendo 3DS is a trademark of Nintendo.

---

## ❤️ Credits and Acknowledgements

Go3DS would not exist without the work of the developers and communities behind the projects it builds upon.

### Godot Engine

[Godot Engine](https://godotengine.org/)

Originally created by Juan Linietsky and Ariel Manzur, and developed by the wider Godot community.

Godot is free and open-source software distributed under the MIT license.

### Original Nintendo 3DS Port

[SeleDreams — godot-4-3ds](https://github.com/SeleDreams/godot-4-3ds)

The original Godot 4 Nintendo 3DS port established the foundation for executing the Godot runtime and GDScript on Nintendo 3DS.

Go3DS continues development from this work.

### Go3DS Dev

[Go3DS-Dev](https://github.com/Go3DS-Dev)

Project direction, development planning, feature requirements, build testing, integration, and ongoing maintenance.

### OpenAI Codex

AI-assisted code generation, implementation, debugging, and development support.

Go3DS openly acknowledges its vibe-coded development process.

### Open-Source Homebrew Community

Special thanks to the developers and contributors behind:

- devkitPro
- devkitARM
- libctru
- Citro2D
- Citro3D
- Other open-source dependencies used by the project

Each dependency remains subject to its own licensing and attribution requirements.

---

## 🤝 Contributing

Go3DS is intended to be an open-source community project.

Contributions are welcome, including:

- Bug reports
- Code reviews
- Renderer improvements
- Performance optimizations
- Hardware compatibility testing
- Documentation
- Example games
- Build system improvements

Because the project is heavily AI-generated, experienced developers reviewing code quality, correctness, and performance are particularly valuable.

If you discover a bug, please include reproduction steps, the hardware or emulator used, and any relevant logs.

Contributions should respect the existing project structure, compatibility goals, and licensing requirements.

---

## 🔗 Links

**Go3DS Engine**

https://github.com/Go3DS-Dev/godot-4-3ds

**Original Nintendo 3DS Port**

https://github.com/SeleDreams/godot-4-3ds

**Godot Engine**

https://godotengine.org/

**devkitPro**

https://devkitpro.org/

**Azahar Emulator**

https://azahar-emu.org/

---

### 🎮 Go3DS Engine

**Make games in Godot. Play them on your Nintendo 3DS.**

*Fully vibe-coded. Open source. Still experimental.*

*Unofficial project — not affiliated with Nintendo or the Godot Foundation.*