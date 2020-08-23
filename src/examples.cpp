#define ERROR_BUFFER_SIZE 1024
#define glPrintErrors() {GLenum err = glGetError(); while(err){LogError("OPENG error code: %x", err);}}

const char *vertexShaderSource = 
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;"
"void main()"
"{"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
"}\0";

const char *fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;"
"void main(){"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);}\0";

internal u32 
ShaderCreate(GLenum shaderType, const char *nullTerminatedShaderFile)
{
    u32 shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &nullTerminatedShaderFile, NULL);
    glCompileShader(shader);
    
    char log[ERROR_BUFFER_SIZE];
    glGetShaderInfoLog(shader, ERROR_BUFFER_SIZE - 1, NULL, log);
    
    char *strShaderType = NULL;
    switch(shaderType)
    {
        case GL_VERTEX_SHADER: strShaderType = "Vertex"; break;
        case GL_GEOMETRY_SHADER: strShaderType = "Geometry"; break;
        case GL_FRAGMENT_SHADER: strShaderType = "Fragment"; break;
    }
    
    i32 status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        LogError("%s shader compilation %s", strShaderType, log);
    }
    else
    {
        LogSuccess("%s shader compiled", strShaderType);
    }
    glPrintErrors();
    
    return shader;
}

internal u32
ProgramCreate(u32 shaders[], u32 shaderCount)
{
    u32 shaderProgram = glCreateProgram();
    
    for(u32 i = 0; i != shaderCount; i++)
        glAttachShader(shaderProgram, shaders[i]);
    
    glLinkProgram(shaderProgram);
    
    i32 status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        char log[ERROR_BUFFER_SIZE];
        glGetProgramInfoLog(shaderProgram, ERROR_BUFFER_SIZE - 1, NULL, log);
        
        LogError("Create program %s", log);
    }
    
    for(u32 i = 0; i != shaderCount; i++)
        glDeleteShader(shaders[i]); 
    
    glPrintErrors();
    
    return shaderProgram;
}

internal void
OpenGLTriangleSetup(void)
{
    
    u32 shaders[2];
    u32 shaderCount = 0;
    
    shaders[shaderCount++] = ShaderCreate(GL_VERTEX_SHADER, vertexShaderSource);
    shaders[shaderCount++] = ShaderCreate(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    u32 shaderProgram = ProgramCreate(shaders, shaderCount);
    
    u32 vertexBufferObject, vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBufferObject); 
    
    f32 vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    }; 
    
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glUseProgram(shaderProgram);
    
    glPrintErrors();
}

#include <math.h>
internal void
AudioGenerateSineWave(void *audioBuffer, i32 sampleCount)
{
    // NOTE: Sine wave controlled by W Key and right controller stick
    i32 toneHz = 261 + (i32)(GetOS()->userInput.controller[0].rightStickX * 100);
    if(IsKeyDown(KEY_W)) toneHz = 350;
    i32 wavePeriod = (48000 / toneHz);
    
    
#define MATH_PI 3.14159265f
    static f32 tSine;
    
    i16 *sample = (i16 *)audioBuffer;
    for(i32 i = 0; i != sampleCount; i++)
    {
        f32 sineValue = sinf(tSine);
        i16 sampleValue = (i16)(sineValue * 3000);
        *sample++ = sampleValue;
        *sample++ = sampleValue;
        
        tSine += 2 * MATH_PI * (f32)1.0f / (f32)wavePeriod;
    }
}

internal i32
RoundF32ToI32(f32 val)
{
    i32 result = (i32)(val + 0.5f);
    
    return result;
}

internal u32
RoundF32ToU32(f32 val)
{
    u32 result = (u32)(val + 0.5f);
    
    return result;
}

internal i32
Clamp(i32 min, i32 val, i32 max)
{
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

internal u32
ColorToU32(v4 color)
{
    u8 alpha = RoundF32ToU32(color.a * 255);
    u8 red =   RoundF32ToU32(color.r * 255);
    u8 green = RoundF32ToU32(color.g * 255);
    u8 blue =  RoundF32ToU32(color.b * 255);
    
    u32 result = (alpha << 24 | red << 16 | green << 8 | blue << 0);
    
    return result;
}

internal void
RenderRectangle(graphics_buffer *buffer, f32 minX, f32 minY, f32 maxX, f32 maxY, v4 color)
{
    i32 x0 = RoundF32ToI32(minX);
    i32 x1 = RoundF32ToI32(maxX);
    i32 y0 = RoundF32ToI32(minY);
    i32 y1 = RoundF32ToI32(maxY);
    
    x0 = Clamp(0, x0, buffer->size.x);
    x1 = Clamp(0, x1, buffer->size.x);
    y0 = Clamp(0, y0, buffer->size.y);
    y1 = Clamp(0, y1, buffer->size.y);
    
    u8 *row = (u8 *)buffer->memory;
    i32 stride = buffer->size.x * buffer->bytesPerPixel;
    row = row + (y0 * stride) + (x0 * buffer->bytesPerPixel);
    
    for(i32 y = y0; y < y1; ++y)
    {
        u32 *pixel = (u32 *)row;
        
        for(i32 x = x0; x < x1; ++x)
        {
            pixel[x] = ColorToU32(color);
        }
        
        row += stride;
    }
}