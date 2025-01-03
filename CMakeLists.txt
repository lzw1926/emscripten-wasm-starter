cmake_minimum_required(VERSION 3.10)
project(webgl-renderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --bind")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lembind -O0 --emit-tsd index.d.ts")

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
                -s EXPORTED_RUNTIME_METHODS=[] \
                -s EXPORTED_FUNCTIONS=[] \
                -s ALLOW_MEMORY_GROWTH=1 \
                -s MODULARIZE=1 \
                -s EXPORT_NAME='initialWasm' \
                -s EXPORT_ES6=1 \
                -s OFFSCREENCANVAS_SUPPORT=1 \
                -s TEXTDECODER=2 \
                -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 \
                -s FILESYSTEM=0 \
                -s ENVIRONMENT='web,worker'"
) 

# -s OFFSCREENCANVAS_SUPPORT=1 \  增加 transferControlToOffscreen 逻辑，配合 explicitSwapControl 使用
# -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 \ 默认值是 1，本意是用新的 DOM 查找 API，不明白为什么要同时去掉 Module.canvas 的使用

# -s MINIMAL_RUNTIME=1 \
# -s EXPORT_KEEPALIVE=1 \
# -s OFFSCREEN_FRAMEBUFFER \ 和 offscreenCanvas support 配合，还不确定用途
# -s TEXTDECODER=2 \ 默认 TextDecoder 可用，不做兜底
# -s BUILD_AS_WORKER=1 \ 默认在 worker 中运行，增加了一些 message 响应逻辑

# lembind 自动数据类型转换 https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#built-in-type-conversions