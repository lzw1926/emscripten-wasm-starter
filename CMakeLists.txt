cmake_minimum_required(VERSION 3.10)
project(webgl-renderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --bind")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lembind")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --emit-tsd")

# Emscripten specific settings
set(CMAKE_EXECUTABLE_SUFFIX ".js")

add_executable(urzx # 目标文件名
    src/renderer.cpp # 源文件
)

# add_compile_options(--emit-tsd)
# Emscripten链接设置
# 第一个参数是目标文件名，和之前add_executable的第一个参数一致
set_target_properties(urzx PROPERTIES
    LINK_FLAGS "-s WASM=1 \
                -s USE_WEBGL2=1 \
                -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS'] \
                -s EXPORTED_FUNCTIONS=['_malloc','_free'] \
                -s ALLOW_MEMORY_GROWTH=1 \
                -s MODULARIZE=1 \
                -s EXPORT_ES6=1 \
                -s ENVIRONMENT='web'"
) 

# -s OFFSCREENCANVAS_SUPPORT=1 \
