/* date = November 20th 2020 3:15 pm */

#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#define ERROR_BUFFER_SIZE 1024
#define GLPrintErrors() PrivateGLPrintErrors(__LINE__, __FUNCTION__, __FILE__) 

void PrivateGLPrintErrors(int line, char *function, char *file);

Internal void
PrivateGLPrintErrors(int line, char *function, char *file)
{
    GLenum err = glGetError(); 
    while(err)
    {
        switch(err)
        {
            case GL_INVALID_ENUM: Log("GL_INVALID_ENUM "); break;
            case GL_INVALID_OPERATION: Log("GL_INVALID_OPERATION "); break;
            case GL_INVALID_VALUE: Log("GL_INVALID_VALUE "); break;
        }
        Log("OPENGL ERROR %s:%d %s CODE: %x\n", file, line, function, err); 
        err = glGetError(); 
        // dbg();
    }
}



static const char *vertexShader = R"SHADER(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 textureCoordinate;
layout (location = 3) in float textureIndex;

out vec4 vertexColor;
out vec2 vertexTextureCoordinate;
out float vertexTextureIndex;

uniform mat4 viewProjectionMatrix;

void main()
{
      gl_Position = vec4(position, 1.0) * viewProjectionMatrix;

    // NOTE: Send to fragment shader
    vertexColor = color;
    vertexTextureCoordinate = textureCoordinate;
    vertexTextureIndex = textureIndex;

}
)SHADER";

static const char *fragmentShader = R"SHADER(
#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec2 vertexTextureCoordinate;
in float vertexTextureIndex;

uniform sampler2D textures[32];

void main()
{

       int textureIndex = int(vertexTextureIndex);
    FragColor = texture(textures[textureIndex], vertexTextureCoordinate) * vertexColor;

}
)SHADER";

struct Texture2D
{
    u32 id;
    i32 width;
    i32 height;
    i32 numberOfChannels;
};

struct Vertex
{
    V3 position;
    V4 color;
    V2 textureCoordinate;
    f32 textureIndex;
};

struct VertexRectangle
{
    Vertex vertices[4];
};

struct ShaderProgram
{
    u32 id;
};

enum DrawCallType
{
    DRAWCALLNull,
    DRAWCALLSprite,
    DRAWCALLColor,
    DRAWCALLText,
};

// TODO(KKrzosa): Allow multiple textures per drawcall, store them in array
// make sure we are not doing more than device limit and then just
// bind those textures to multiple active textures and draw at once
// not really sure anymore if the indexes in vertex buffer is good anymore
// we will have to then update the vertices texture indexes before drawing
// or perhaps update that when push draw call happens from 1 ... limit
struct DrawCall
{
    DrawCallType type;
    Texture2D texture;
    u32 dataSize;
    u32 dataOffset;
};


#define MAX_QUAD_COUNT 65536
#define MAX_DRAW_CALLS 65536
#define MAX_TEXTURE_COUNT 128
#define ELEMENTS_PER_QUAD 6

#define MAX_VERTEX_COUNT MAX_QUAD_COUNT * 4
#define ELEMENT_COUNT ELEMENTS_PER_QUAD * MAX_QUAD_COUNT
#define TEXTURE_ARRAY_SIZE_IN_SHADER 32

struct OpenGLRenderer
{
    ShaderProgram basicShader;
    
    f32 width;
    f32 height;
    u32 vertexBufferIndex; 
    u32 vertexArrayIndex;
    u32 elementBufferIndex;
    
    // Query string opengl info about device
    i32 MAX_TEXTURE_IMAGE_UNITS;
    i32 MAX_COMBINED_TEXTURE_IMAGE_UNITS;
    
    
    i32 textureSlots[TEXTURE_ARRAY_SIZE_IN_SHADER];
    Texture2D whiteTexture;
    
    u32 texturesOnGPU[MAX_TEXTURE_COUNT];
    u32 textureCount;
    
    DrawCall activeDrawCall;
    DrawCall drawCalls[MAX_DRAW_CALLS];
    u32 drawCallCount;
    
    
    u32 elements[ELEMENT_COUNT];
    VertexRectangle quadBuffer[MAX_QUAD_COUNT]; 
    u32 currentQuadCount;
};


Internal void OpenGLRendererAttach(OpenGLRenderer *gl);
Internal OpenGLRenderer *OpenGLRendererGet();

#endif //OPENGL_RENDERER_H
