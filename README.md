# Pilot Engine

<p align="center">
  <a href="https://games104.boomingtech.com">
    <img src="engine/source/editor/resource/PilotEngine.png" width="400" alt="Pilot Engine logo">
  </a>
</p>

**Pilot Engine** is a tiny game engine used for the [GAMES104](https://games104.boomingtech.com) course.

## Prerequisites

To build Pilot, you must first install the following tools.

### Windows 10/11
- Visual Studio 2019 (or more recent)
- CMake 3.19 (or more recent)
- Git 2.1 (or more recent)

### MacOS >= 10.15 (x86_64)
- Xcode 12.3 (or more recent)
- CMake 3.19 (or more recent)
- Git 2.1 (or more recent)

### Ubuntu 20.04
 - apt install the following packages
```
sudo apt install git
sudo apt install cmake
sudo apt install clang
sudo apt install libxcb1-dev
sudo apt install libvulkan-dev
sudo apt install vulkan-validationlayers
sudo apt install mesa-vulkan-drivers
```  
- [NVIDIA driver](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#runfile) (The AMD and Intel driver is open-source, and thus is installed automatically by mesa-vulkan-drivers)

## Build Pilot

### Build on Windows
You may execute the **build_windows.bat**. This batch file will generate the projects, and build the **Release** config of **Pilot Engine** automatically. After successful build, you can find the PilotEditor.exe at the **bin** directory.

Or you can use the following command to generate the **Visual Studio** project firstly, then open the solution in the build directory and build it manually.
```
cmake -S engine/ -B build
```

### Build on MacOS

> The following build instructions only tested on specific hardware of x86_64, and do not support M1 chips. For M1 compatible, we will release later.

To compile Pilot, you must have the most recent version of Xcode installed.
Then run 'cmake' from the project's root directory, to generate a project of Xcode.

```
cmake -S engine/ -B build -G "Xcode"
```
and you can build the project with 
```
cmake --build build --config Release
```

### Build on Ubuntu 20.04 
You can execute the **build_linux.sh** to build the binaries.  
  
