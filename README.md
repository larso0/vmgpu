# vmgpu
Vulkan Multi-GPU application
This program visualizes a single rotating mesh loaded from an obj file, using multiple GPUs with sort-first or sort-last approaches. It provides a command line interface for passing options. run vmgpu --help for more info. This depends on my abstraction library for Vulkan, bp.

## Building
In order to build vmgpu you need boost (for program_options and filesystem), Qt (at least 5.10), and Vulkan SDK (or the appropriate vulkan package from your package manager of choice).

    git clone --recurse-submodules https://github.com/larso0/vmgpu.git $SOURCE_DIR
    cd $SOURCE_DIR/bp/external/shaderc/third_party
    git clone https://github.com/google/googletest.git
    git clone https://github.com/google/glslang.git
    git clone https://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
    git clone https://github.com/KhronosGroup/SPIRV-Headers.git spirv-headers
    cd $SOURCE_DIR
    cd $BUILD_DIR
    cmake <options> $SOURCE_DIR

It is possible to build with Visual Studio, though I have had little success in compiling and linking boost with Visual Studio, so that is left as an exercise for the user. I recommend either using linux, or an MSYS2 environment on windows (mingw-w64 compiler), where boost, Qt and vulkan packages are available from the package manager.
