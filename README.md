2D stars with:
- [elastic collisions](https://en.wikipedia.org/wiki/Elastic_collision)
- basic gravity
- physics that are unaffected by frame rate unless it drops below 30 frames per second
- random star generation within set parameters (tips, size, color, fill draw, line draw, etc.)
- a preview that displays min, average, and max star instances based on the generation parameters (so that you have an idea what star shapes will be generated before you actually generate instances)
- save system for user settings
- color-related:
    - a few color shifting modes (iterating through an array of rainbow colors at specified rate and applying those colors to stars)
    - blending
- simple "trails" achieved by disabling screen clearing
- a few keyboard shortcuts for convenience:
    - `Esc` closes the focused window; if the focused window is the main window, it will close the program
    - `A` clears main window once
    - `S` toggles main window clearing (this enables/disables "trails")
    - `D` toggles config window
    - `F` toggles fullscreen on focused window
- most settings/features are toggleable/adjustable

***

This program was written with tools from the compiler environment provided by [WinLibs](https://winlibs.com/) (specifically: `clang++`/`g++` for `C++20`, `gdb`,  `clang-format`, and `clang-tidy`), the [VSCode](https://code.visualstudio.com/) editor, and the [C/C++ VSCode extension](https://github.com/Microsoft/vscode-cpptools). 

The following libraries were used:
- [OpenGL 4.6](https://www.opengl.org/) for graphics.
- [GLFW 3.3.2](https://www.glfw.org/) for window management.
- [Glad 2 in core configuration for OpenGL 4.6](https://github.com/Dav1dde/glad/tree/glad2) as the [OpenGL Loader Library](https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library).
- [GLM 0.9.9.8](https://github.com/g-truc/glm) for matrix operations (projection, translation, rotation, etc.) and math constants.
- [Dear ImGui 1.87](https://github.com/ocornut/imgui) for the GUI.
- [Boost 1.79.0](https://www.boost.org/) for their filesystem and serialization functionality.

Further credits:
- [Wikipedia](https://www.wikipedia.org/) for the mathematical [circle-circle elastic collision response formula](https://en.wikipedia.org/wiki/Elastic_collision#Two-dimensional_collision_with_two_moving_objects).
- [LearnOpenGL](https://learnopengl.com/) for an excellent tutorial on OpenGL.
- [Desmos](https://www.desmos.com/) for useful graphing functionality (which allowed me to visualize [overlap correction](https://www.desmos.com/calculator/zcg3yrwuj4)).
- [Draw.io](https://github.com/jgraph/drawio-desktop) for a useful diagramming tool.

See **[LICENSE.md](LICENSE.md)** for information on the various licenses relating to this project.

***

Example environment setup for Windows 10:
- download `GCC 12.2.0 + LLVM/Clang/LLD/LLDB 14.0.6 + MinGW-w64 10.0.0 (UCRT) - release 2` for `Win64` from [WinLibs](https://winlibs.com/)
- download and [compile GLFW 3.3.2](https://www.glfw.org/docs/3.3/compile.html)
    - short instructions (will need [cmake](https://cmake.org/download/)):
        - `cd GLFW_ROOT` (where `GLFW_ROOT` is the GLFW directory containing CMakeLists.txt, should be the toplevel of the GLFW folder)
        - `mkdir build`
        - `cd build`
        - `cmake -G "MinGW Makefiles" -S GLFW_ROOT` (`-G` specify generator, must match MinGW; `-S` path to `GLFW_ROOT` with CMakeLists.txt as before)
        - `mingw32-make`
        - `GLFW_ROOT\build\src` contains the compiled library
- generate `Glad 2` in core configuration for `OpenGL 4.6` and download
- download `GLM 0.9.9.8`
- download `Dear ImGui 1.87`
- download and [compile Boost 1.79.0](https://www.boost.org/doc/libs/1_79_0/more/getting_started/windows.html#prepare-to-use-a-boost-library-binary)
    - short instructions (this will compile every library which takes a fairly long time):
        - `cd BOOST_ROOT\tools\build`
        - `bootstrap.bat gcc` (`gcc` so we can take advantage of the already installed MinGW)
        - `cd BOOST_ROOT`
        - `BOOST_ROOT\tools\build\b2 toolset=gcc` (use the newly built `tools\build\b2.exe` to build the Boost libraries)
        - `BOOST_ROOT\stage\lib` contains the compiled libraries
    - find `libboost_filesystem-mgw12-mt-x64-1_79.dll` and `libboost_serialization-mgw12-mt-x64-1_79.dll` in `BOOST_ROOT\stage\lib` and copy them into a folder entitled `compiled` located the same directory as the `source` folder
        - these DLLs are necessary for the compiled executable to run (unless you statically link), if you don't do this step, you will get "DLL not found" errors

For this example, a `compile.bat` file has been included in this repository. If you want to actually develop with this setup, you should configure an editor instead.

To use `compile.bat`:
- Adjust the paths within `compile.bat` to match the directory locations that you have used for the above environment software.
    - A few notes on the `.bat` file in case you are unfamiliar:
        - `if not exist "compiled" mkdir "compiled"` creates a folder entitled "compiled" in the current directory if a folder of that name does not already exist.
        - The `^` symbol indicates a line break (like `\` in some programming languages) and is used for readability. When the .bat file is executed, any line ending with `^` will have the next line appended to it. Make sure you never end a file with a `^` as it, apparently, will quickly consume all available memory of the system.
        - Consult [clang++](https://clang.llvm.org/docs/ClangCommandLineReference.html) and/or [g++](https://gcc.gnu.org/onlinedocs/gcc/Invoking-GCC.html) documentation or Google for information on compiler switches.
- After adjusting `compile.bat`, run it. If it is configured correctly, it will create an executable entitled `stars.exe` in the `compiled` folder.
- Execute `stars.exe`.
    - Note that `stars.exe` will create a folder entitled `config` in which various user config files are stored.
        - This occurs when the user saves a config and when the program is closed.
        - Config saving will overwrite without prompt.
    - If the program immediately closes on execution, run it from `cmd` or `powershell`. If it closed due to a lack of driver `OpenGL 4.6` support, it should display a corresponding error message.