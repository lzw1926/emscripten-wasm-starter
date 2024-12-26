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

    // 设置视口
    glViewport(0, 0, width, height);
    
    // 初始化着色器
    initShaders();
    
    // 初始化顶点缓冲
    initBuffers();
    
    // 初始化纹理
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ImageRenderer::loadAndRender(std::string imageData, int imageWidth, int imageHeight) {
    // 计算适配后的尺寸和偏移
    float scaleX = static_cast<float>(canvasWidth) / imageWidth;
    float scaleY = static_cast<float>(canvasHeight) / imageHeight;
    float scale = std::min(scaleX, scaleY);  // 取最小缩放比以适应容器
    
    float scaledWidth = imageWidth * scale;
    float scaledHeight = imageHeight * scale;
    
    // 计算归一化坐标（-1到1范围）
    float normalizedWidth = (scaledWidth / canvasWidth) * 2.0f;
    float normalizedHeight = (scaledHeight / canvasHeight) * 2.0f;
    
    // 计算居中偏移
    float offsetX = (2.0f - normalizedWidth) / 2.0f;
    float offsetY = (2.0f - normalizedHeight) / 2.0f;
    
    // 更新顶点数据，翻转纹理坐标的Y值
    float vertices[] = {
        // 位置                                  // 纹理坐标 (Y从0改为1，从1改为0)
        -1.0f + offsetX, 1.0f - offsetY,        0.0f, 0.0f,  // 左上
        -1.0f + offsetX, 1.0f - offsetY - normalizedHeight,  0.0f, 1.0f,  // 左下
         -1.0f + offsetX + normalizedWidth, 1.0f - offsetY,  1.0f, 0.0f,  // 右上
         -1.0f + offsetX + normalizedWidth, 1.0f - offsetY - normalizedHeight, 1.0f, 1.0f   // 右下
    };
    
    // 更新顶点缓冲
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 激活纹理单元0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // 上传纹理数据
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0,                    
        GL_RGBA,             
        imageWidth,          // 使用实际图片宽度
        imageHeight,         // 使用实际图片高度
        0,                    
        GL_RGBA,             
        GL_UNSIGNED_BYTE,    
        imageData.data()
    );
    
    // 检查错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("GL error after texture upload: %d\n", error);
        return;
    }

    // 清除并渲染
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(shaderId);
    
    GLint texLocation = glGetUniformLocation(shaderId, "uTexture");
    glUniform1i(texLocation, 0);
    
    GLint posAttrib = glGetAttribLocation(shaderId, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    GLint texCoordAttrib = glGetAttribLocation(shaderId, "texCoord");
    glEnableVertexAttribArray(texCoordAttrib);
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                         (void*)(2 * sizeof(float)));
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glDisableVertexAttribArray(posAttrib);
    glDisableVertexAttribArray(texCoordAttrib);
}

void ImageRenderer::initShaders() {
    // 创建并编译顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // 检查顶点着色器编译状态
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Vertex shader compilation failed: %s\n", infoLog);
    }

    // 创建并编译片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    // 检查片段着色器编译状态
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Fragment shader compilation failed: %s\n", infoLog);
    }

    // 创建着色器程序并链接
    shaderId = glCreateProgram();
    glAttachShader(shaderId, vertexShader);
    glAttachShader(shaderId, fragmentShader);
    glLinkProgram(shaderId);
    
    // 检查程序链接状态
    glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderId, 512, NULL, infoLog);
        printf("Shader program linking failed: %s\n", infoLog);
    }

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
}

EMSCRIPTEN_BINDINGS(image_renderer_module) {
    emscripten::class_<ImageRenderer>("ImageRenderer")
        .constructor()
        .function("initGL", &ImageRenderer::initGL)
        .function("loadAndRender", &ImageRenderer::loadAndRender);
    
}