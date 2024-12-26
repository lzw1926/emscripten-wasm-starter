#include "renderer.hpp"
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <emscripten/html5.h>

// 顶点着色器
const char* vertexShaderSource = R"(
    attribute vec2 position;
    attribute vec2 texCoord;
    varying vec2 vTexCoord;
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
        vTexCoord = texCoord;
    }
)";

// 片段着色器
const char* fragmentShaderSource = R"(
    precision mediump float;
    varying vec2 vTexCoord;
    uniform sampler2D uTexture;
    void main() {
        gl_FragColor = texture2D(uTexture, vTexCoord);
    }
)";

ImageRenderer::ImageRenderer() : 
    textureId(0), 
    shaderId(0), 
    vboId(0),
    canvasWidth(0),
    canvasHeight(0) {}

ImageRenderer::~ImageRenderer() {
    if (textureId) glDeleteTextures(1, &textureId);
    if (shaderId) glDeleteProgram(shaderId);
    if (vboId) glDeleteBuffers(1, &vboId);
}

void ImageRenderer::initGL(std::string canvasId, int width, int height) {
    canvasWidth = width;
    canvasHeight = height;
    
    // 创建WebGL上下文
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.enableExtensionsByDefault = 1;
    attrs.alpha = 1;
    attrs.depth = 1;
    attrs.stencil = 1;
    attrs.antialias = 1;
    attrs.premultipliedAlpha = 1;
    attrs.preserveDrawingBuffer = 0;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    attrs.failIfMajorPerformanceCaveat = 0;
    
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(canvasId.c_str(), &attrs);
    if (context <= 0) {
        printf("Failed to create WebGL context!\n");
        return;
    }
    
    // 设置当前上下文
    emscripten_webgl_make_context_current(context);
    
    // 初始化着色器
    initShaders();
    
    // 初始化顶点缓冲
    initBuffers();
    
    // 初始化纹理
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ImageRenderer::loadAndRender(std::string imageData) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // 上传纹理数据
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA, 
        canvasWidth, 
        canvasHeight, 
        0, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        imageData.data()
    );
    
    // 渲染
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ImageRenderer::initShaders() {
    // 创建并编译顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // 创建并编译片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // 创建着色器程序并链接
    shaderId = glCreateProgram();
    glAttachShader(shaderId, vertexShader);
    glAttachShader(shaderId, fragmentShader);
    glLinkProgram(shaderId);

    // 清理着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ImageRenderer::initBuffers() {
    // 定义顶点数据（位置和纹理坐标）
    float vertices[] = {
        // 位置     // 纹理坐标
        -1.0f,  1.0f,  0.0f, 0.0f, // 左上
        -1.0f, -1.0f,  0.0f, 1.0f, // 左下
         1.0f,  1.0f,  1.0f, 0.0f, // 右上
         1.0f, -1.0f,  1.0f, 1.0f  // 右下
    };

    // 创建并绑定顶点缓冲对象
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性
    GLint posAttrib = glGetAttribLocation(shaderId, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    GLint texCoordAttrib = glGetAttribLocation(shaderId, "texCoord");
    glEnableVertexAttribArray(texCoordAttrib);
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                         (void*)(2 * sizeof(float)));
}

EMSCRIPTEN_BINDINGS(image_renderer_module) {
    emscripten::class_<ImageRenderer>("ImageRenderer")
        .constructor()
        .function("initGL", &ImageRenderer::initGL)
        .function("loadAndRender", &ImageRenderer::loadAndRender);
    
}