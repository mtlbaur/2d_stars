mkdir bin
cd bin
mkdir debug
mkdir release
cd ..
robocopy '../../libraries/boost_1_79_0/stage/lib' 'bin/debug' 'libboost_filesystem-mgw12-mt-x64-1_79.dll' 'libboost_serialization-mgw12-mt-x64-1_79.dll'
robocopy '../../libraries/boost_1_79_0/stage/lib' 'bin/release' 'libboost_filesystem-mgw12-mt-x64-1_79.dll' 'libboost_serialization-mgw12-mt-x64-1_79.dll'
mkdir build
cd build
mkdir debug
mkdir release
cd debug
cmake -G 'Ninja' -S '../..' -DCMAKE_BUILD_TYPE=Debug -DCMAKE_RUNTIME_OUTPUT_DIRECTORY='../../bin/debug'
ninja # compile immediately
cd ..
cd release
cmake -G 'Ninja' -S '../..' -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY='../../bin/release'
ninja # compile immediately
cd '../..'