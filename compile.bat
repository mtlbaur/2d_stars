if not exist "compiled" mkdir "compiled"

..\..\environments\i686-8.1.0-release-posix-dwarf-rt_v6-rev0\mingw32\bin\g++.exe ^
source\*.cpp ^
-fdiagnostics-color=always ^
-std=c++17 ^
-Wall ^
-O3 ^
..\..\libraries\imgui-1.87\imgui*.cpp ^
..\..\libraries\imgui-1.87\backends\imgui_impl_glfw.cpp ^
..\..\libraries\imgui-1.87\backends\imgui_impl_opengl3.cpp ^
..\..\libraries\glad2_core\src\gl.c ^
-I..\..\libraries\imgui-1.87 ^
-I..\..\libraries\imgui-1.87\backends ^
-I..\..\libraries\glad2_core\include ^
-I..\..\libraries\glfw-3.3.2\include ^
-I..\..\libraries\glm-0.9.9.8\glm ^
-I..\..\libraries\boost_1_79_0 ^
-L..\..\libraries\glfw-3.3.2\build\src ^
-L..\..\libraries\boost_1_79_0\stage\lib ^
-lglfw3 ^
-lgdi32 ^
-lboost_filesystem-mgw8-mt-x32-1_79 ^
-lboost_serialization-mgw8-mt-x32-1_79 ^
-ocompiled\stars.exe