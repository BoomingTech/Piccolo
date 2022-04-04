# Simple .obj viewer with glew + glfw3 + OpenGL

## Requirements

* premake5
* glfw3
* glew
* xcursor
* xinerama

## Build on MaCOSX

Install glfw3 and glew using brew.
Then,

    $ premake5 gmake
    $ make

## Build on Linux

Set `PKG_CONFIG_PATH` or Edit path to glfw3 and glew in `premake4.lua`

Then,

    $ premake5 gmake
    $ make

## Build on Windows.

* Visual Studio 2013
* Windows 64bit
  * 32bit may work.

Put glfw3 and glew library somewhere and replace include and lib path in `premake4.lua`

Then,

    > premake5.exe vs2013

## TODO

* [ ] Alpha texturing.
* [ ] Support per-face material.
* [ ] Use shader-based GL rendering.
* [ ] PBR shader support.
