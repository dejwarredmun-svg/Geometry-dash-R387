# R387 Agent

**Geometry Dash image-to-object generator · by Dejwar387**

R387 Agent is a Windows Geode mod project for Geometry Dash 2.2081 / Geode v5.11.1. It decodes local images, converts them to a capped grid of colored, editable Geometry Dash objects, and uses the current editor selection as the placement reference. The basic conversion runs locally and does not need an AI key, internet access, or image upload.

> **Development status:** the portable image-conversion engine is implemented and can be tested independently. The Geode/editor integration is source-complete but has **not** been compiled or tested in Geometry Dash in this environment because Geode CLI/SDK are not installed here. Do not treat this archive as a ready-to-install `.geode` file.

## Current implementation

- Adds an **R387** button in the Geometry Dash editor.
- Opens a custom dark, two-column R387 Agent popup.
- Reads the current single/multiple editor selection and displays the selection count.
- Uses a Windows file dialog to load PNG, JPEG/JPG, and BMP files locally through GDI+.
- Rejects invalid images and dimensions above 4096 × 4096.
- Shows a live color-grid preview before insertion.
- Supports object budget presets from 1 to 2500 and a separate **Unlimited** mode.
- Applies an actual insertion-boundary check so the output cannot exceed the selected/safety budget.
- Skips transparent samples, quantizes colors, and adapts the grid dimensions to the source image aspect ratio.
- Adds generated objects without moving or deleting the original selection.

## Important limitations of this first build

- **Not yet verified against the live Geode SDK/game.** CMake configuration and in-game integration still need validation on a Windows machine with the supported SDK.
- Unlimited is deliberately not infinite: it uses a hard safety cap of 2500 objects to avoid a runaway level/editor workload.
- The initial renderer creates colored object-ID-1 blocks. Those are editable, but they may collide during gameplay; use this version for background/deco areas until a non-colliding decoration-object mapping is added.
- The image is currently represented by one color sample per grid cell. This is a pixel-art renderer, not a machine-learning model; results depend on image resolution and object budget.
- File selection is implemented; direct Explorer drag-and-drop is not yet wired into the game's window.
- Undo grouping, cancelable frame-batched generation, geometric/edge modes, transparency refinement, and a custom editor-object palette are not implemented yet.
- The current popup uses a budget stepper rather than a continuous draggable slider.
- Actual compilation, Geode load, generated-object placement, and level-save behavior remain unverified until tested in Geometry Dash.

## Project structure

```text
R387-Agent/
├── .github/workflows/build.yml
├── assets/logo.svg
├── src/
│   ├── EditorIntegration.cpp
│   ├── ImageProcessor.cpp
│   ├── ImageProcessor.hpp
│   ├── R387Popup.cpp
│   ├── R387Popup.hpp
│   ├── WindowsImageLoader.cpp
│   └── WindowsImageLoader.hpp
├── tests/core_tests.cpp
├── tools/run_core_tests.ps1
├── CMakeLists.txt
├── LICENSE
├── README.md
└── mod.json
```

## Requirements for a real `.geode` build

1. Windows 10/11, a compatible Geometry Dash install, Git, CMake 3.21+, and a supported C++ toolchain (Visual Studio Build Tools 2022 is recommended).
2. Install Geode CLI from the official documentation: <https://docs.geode-sdk.org/getting-started/geode-cli/>
3. Open a fresh PowerShell window and run:

   ```powershell
   geode --version
   geode sdk install
   geode sdk install-binaries
   ```

4. Set up a Geode profile if you want `geode build` to install automatically:

   ```powershell
   geode config setup
   ```

5. Open PowerShell in this project folder and build:

   ```powershell
   cd "$env:USERPROFILE\Downloads\R387-Agent"
   geode build
   ```

   If `geode build` reports that CMake or a compiler is missing, install CMake and Visual Studio Build Tools, then reopen PowerShell. If the SDK or Geode version differs from the manifest, update the exact target version in `mod.json` to match the SDK you installed.

6. A successful build should create `dejwar387.r387-agent.geode` in the generated build directory. The exact location depends on the installed Geode CLI version/configuration. Only treat it as ready after the command reports a successful package build.

## Install the `.geode` file

Use the Geode mod manager's supported install flow for your version, or copy the successfully built `.geode` file into the active Geode profile's `mods` directory. Restart/reload Geometry Dash and confirm **R387 Agent** appears in the Geode mod list. Keep a backup of any level before testing object generation.

## Run portable converter tests

These tests cover the non-game-specific conversion/budget engine and can run on a PC even without Geode installed.

PowerShell on Windows:

```powershell
.\tools\run_core_tests.ps1
```

Or with CMake directly:

```powershell
cmake -S . -B build-core-tests -DR387_CORE_TESTS_ONLY=ON
cmake --build build-core-tests --config Release
ctest --test-dir build-core-tests -C Release --output-on-failure
```

They verify that a low object cap is respected, a budget of 1 produces at most one cell, transparent samples produce no objects, unlimited mode is bounded, and invalid images fail safely.

## Publishing on GitHub

The folder is structured as a Git repository root. To publish it after reviewing the code:

```powershell
git init
git add .
git commit -m "Initial R387 Agent implementation"
git branch -M main
git remote add origin https://github.com/YOUR-ACCOUNT/R387-Agent.git
git push -u origin main
```

Create the empty `R387-Agent` repository on GitHub first and replace `YOUR-ACCOUNT`. No GitHub repository or release has been created by this local build step.

## License

Project source code is licensed under MIT. Geode and Geometry Dash remain third-party projects with their own licenses and terms. The SVG logo is original for this project.
