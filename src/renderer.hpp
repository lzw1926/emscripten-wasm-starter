#pragma once
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>

class ImageRenderer {
    public:
        ImageRenderer();
        ~ImageRenderer();
        
        // 初始化WebGL上下文
        void initGL(std::string canvasId, int width, int height);
        
        // 加载并渲染图片
        void loadAndRender(std::string imageData, int imageWidth, int imageHeight);
        
    private:
        // GL相关变量
        unsigned int textureId;
        unsigned int shaderId;
        unsigned int vboId;
        
        int canvasWidth;
        int canvasHeight;
        
        // 着色器初始化
        void initShaders();
        // 初始化顶点数据
        void initBuffers();
}; 