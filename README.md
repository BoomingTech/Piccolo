# Piccolo Engine (formerly Pilot Engine)

<p align="center">
  <a href="https://games104.boomingtech.com">
    <img src="engine/source/editor/resource/PiccoloEngine.png" width="400" alt="Piccolo Engine logo">
  </a>
</p>

**Piccolo Engine** is a tiny game engine used for the [GAMES104](https://games104.boomingtech.com) course.

## Continuous build status

|    Build Type     |                                                                                      Status                                                                                      |
| :---------------: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
| **Build Windows** | [![Build Windows](https://github.com/BoomingTech/Piccolo/actions/workflows/build_windows.yml/badge.svg)](https://github.com/BoomingTech/Piccolo/actions/workflows/build_windows.yml) |
|  **Build Linux**  |    [![Build Linux](https://github.com/BoomingTech/Piccolo/actions/workflows/build_linux.yml/badge.svg)](https://github.com/BoomingTech/Piccolo/actions/workflows/build_linux.yml)    |
|  **Build macOS**  |    [![Build macOS](https://github.com/BoomingTech/Piccolo/actions/workflows/build_macos.yml/badge.svg)](https://github.com/BoomingTech/Piccolo/actions/workflows/build_macos.yml)    |

## Prerequisites

To build Piccolo, you must first install the following tools.

### Windows 10/11
- Visual Studio 2019 (or more recent)
- CMake 3.19 (or more recent)
- Git 2.1 (or more recent)

### macOS >= 10.15 (x86_64)
- Xcode 12.3 (or more recent)
- CMake 3.19 (or more recent)
- Git 2.1 (or more recent)

### Ubuntu 20.04
 - apt install the following packages
```
sudo apt install libxrandr-dev
sudo apt install libxrender-dev
sudo apt install libxinerama-dev
sudo apt install libxcursor-dev
sudo apt install libxi-dev
sudo apt install libglvnd-dev
sudo apt install libvulkan-dev
sudo apt install cmake
sudo apt install clang
sudo apt install libc++-dev
sudo apt install libglew-dev
sudo apt install libglfw3-dev
sudo apt install vulkan-validationlayers
sudo apt install mesa-vulkan-drivers
```
- [NVIDIA driver](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#runfile) (The AMD and Intel driver is open-source, and thus is installed automatically by mesa-vulkan-drivers)

## Build Piccolo

### Build on Windows
You may execute the **build_windows.bat**. This batch file will generate the projects, and build the **Release** config of **Piccolo Engine** automatically. After successful build, you can find the PiccoloEditor.exe at the **bin** directory.

Or you can use the following command to generate the **Visual Studio** project firstly, then open the solution in the build directory and build it manually.
```
cmake -S . -B build
```

### Build on macOS

> The following build instructions only tested on specific hardware of x86_64, and do not support M1 chips. For M1 compatible, we will release later.

To compile Piccolo, you must have the most recent version of Xcode installed.
Then run 'cmake' from the project's root directory, to generate a project of Xcode.

```
cmake -S . -B build -G "Xcode"
```
and you can build the project with
```
cmake --build build --config Release
```

Or you can execute the **build_macos.sh** to build the binaries.

### Build on Ubuntu 20.04
You can execute the **build_linux.sh** to build the binaries.

## Documentation
For documentation, please refer to the Wiki section.

## Extra

### Vulkan Validation Layer: Validation Error
We have noticed some developers on Windows encounted PiccoloEditor.exe could run normally but reported an exception Vulkan Validation Layer: Validation Error
when debugging. You can solve this problem by installing Vulkan SDK (official newest version will do).

### Generate Compilation Database

You can build `compile_commands.json` with the following commands when `Unix Makefiles` generaters are avaliable. `compile_commands.json` is the file
required by `clangd` language server, which is a backend for cpp lsp-mode in Emacs.

For Windows:

``` powershell
cmake -DCMAKE_TRY_COMPILE_TARGET_TYPE="STATIC_LIBRARY" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B compile_db_temp -G "Unix Makefiles"
copy compile_db_temp\compile_commands.json .
```

### Using Physics Debug Renderer
Currently Physics Debug Renderer is only available on Windows. You can use the following command to generate the solution with the debugger project.

``` powershell
cmake -S . -B build -DENABLE_PHYSICS_DEBUG_RENDERER=ON
```

Note:
1. Please clean the build directory before regenerating the solution. We've encountered building problems in regenerating directly with previous CMakeCache.
2. Physics Debug Renderer will run when you start PiccoloEditor. We've synced the camera position between both scenes. But the initial camera mode in Physics Debug Renderer is wrong. Scrolling down the mouse wheel once will change the camera of Physics Debug Renderer to the correct mode.
