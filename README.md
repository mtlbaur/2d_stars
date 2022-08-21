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
    - `A` toggles main window clearing (this enables/disables "trails")
    - `S` clears main window once
    - `D` toggles config window
    - `F` toggles fullscreen on focused window
- most settings/features are toggleable/adjustable

***

This program was written for Windows 10 and compiled with [MinGW-w64 8.1.0](https://www.mingw-w64.org/)'s `g++` for C++17. The following software was used:
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
- [Draw.io](https://github.com/jgraph/drawio-desktop) for a useful diagramming tool (see [this image](images/diagram.png)).

See **[LICENSE.md](LICENSE.md)** for information on the various licenses relating to this project.

***

Preparing the environment on Windows 10:

- download MinGW-w64 8.1.0 (you can find the different MinGW releases [here](https://sourceforge.net/projects/mingw-w64/files/mingw-w64/); i686-posix-dwarf was used for this program)
- download and [compile GLFW 3.3.2](https://www.glfw.org/docs/3.3/compile.html)
    - short instructions (will need [cmake](https://cmake.org/download/)):
        - `cd GLFW_ROOT` (where `GLFW_ROOT` is the GLFW directory containing CMakeLists.txt, should be the toplevel of the GLFW folder)
        - `mkdir build`
        - `cd build`
        - `cmake -G "MinGW Makefiles" -S GLFW_ROOT` (`-G` specify generator, must match MinGW; `-S` path to `GLFW_ROOT` with CMakeLists.txt as before)
        - `mingw32-make`
        - `GLFW_ROOT\build\src` contains the compiled library
- generate Glad 2 in core configuration for OpenGL 4.6 and download
- download GLM 0.9.9.8
- download Dear ImGui 1.87
- download and [compile Boost 1.79.0](https://www.boost.org/doc/libs/1_79_0/more/getting_started/windows.html#prepare-to-use-a-boost-library-binary)
    - short instructions (this will compile every library which takes a fairly long time):
        - `cd BOOST_ROOT\tools\build`
        - `bootstrap.bat gcc` (`gcc` so we can take advantage of the already installed MinGW)
        - `cd BOOST_ROOT`
        - `BOOST_ROOT\tools\build\b2 toolset=gcc` (use the newly built `tools\build\b2.exe` to build the Boost libraries)
        - `BOOST_ROOT\stage\lib` contains the compiled libraries

***

Compiling/running on Windows 10:

- Adjust the paths within compile.bat (included in this repository) to match the directory locations that you have used for the above environment software.
    - A few notes on the .bat file in case you are unfamiliar:
        - `if not exist "compiled" mkdir "compiled"` creates a folder entitled "compiled" in the current directory if a folder of that name does not already exist.
        - The `^` symbol indicates a line break (like `\` in some programming languages) and is used for readability. When the .bat file is executed, any line ending with `^` will have the next line appended to it. Make sure you never end a file with a `^` as it, apparently, will quickly consume all available memory of the system.
        - Consult [documentation](https://gcc.gnu.org/onlinedocs/gcc/Invoking-GCC.html) or Google for information on the `g++` switches.
- After adjusting compile.bat, run it. If it is configured correctly, it will create an executable entitled "stars.exe" in the "compiled" folder.
- Execute stars.exe.
    - Note that stars.exe will create a folder entitled "config" in which various user config files are stored.
        - This occurs when the user saves a config and when the program is closed.
        - Config saving will overwrite without prompt.
    - If the program immediately closes on execution, run it from cmd or powershell. If it closed due to a lack of driver OpenGL 4.6 support, it should display a corresponding error message.