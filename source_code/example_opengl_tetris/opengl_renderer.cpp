#define ERROR_BUFFER_SIZE 1024
#define GLPrintErrors() PrivateGLPrintErrors(__LINE__, __FUNCTION__, __FILE__) 

void PrivateGLPrintErrors(int line, char *function, char *file);

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

struct texture2d
{
    u32 id;
    i32 width;
    i32 height;
    i32 numberOfChannels;
};

struct vertex
{
    v3 position;
    v4 color;
    v2 textureCoordinate;
    f32 textureIndex;
};

struct vertex_rectangle
{
    vertex vertices[4];
};

struct shader_program
{
    u32 id;
};

static const i32 maxQuadCount = 255;
static const i32 maxVertexCount = maxQuadCount * 4; 
static const i32 elementsPerQuad = 6;
static const i32 maxTextureCount = 128;

struct opengl_renderer
{
    shader_program basicShader;

    u32 vertexBufferIndex; 
    u32 vertexArrayIndex;
    u32 elementBufferIndex;

    i32 deviceTextureUnitCount;
    i32 textureSlots[32];

    texture2d textures[maxTextureCount];
    i32 textureCount;

    u32 elementArray[elementsPerQuad * maxQuadCount];
    vertex_rectangle quadArray[maxQuadCount];
    i32 currentQuadCount;

    camera2d camera;
};
global opengl_renderer *gl;

internal i32
ShaderGetUniformLocation(shader_program shader, char *uniformName)
{
    i32 uniformLocation = glGetUniformLocation(shader.id, uniformName);
    if(uniformLocation != -1)
    {
        return uniformLocation;
    }
    else
    {
        LogError("UniformLocation invalid uniform name");
        dbg;
        return -1;
    }
}

internal void 
ShaderUniform(shader_program shader, char *uniformName, i32 value)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniform1i(uniformLocation, value);
}

internal void 
ShaderUniform(shader_program shader, char *uniformName, m4x4 matrix)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &matrix.e[0][0]);
}

internal void 
ShaderUniform(shader_program shader, char *uniformName, i32 value[], i32 count)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniform1iv(uniformLocation, count, value);
}

internal void 
ShaderUniform(shader_program shader, char *uniformName, v4 vector)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniform4f(uniformLocation, vector.x, vector.y, vector.z, vector.w);
}

internal void
ShaderUse(shader_program shader)
{
    glUseProgram(shader.id);
}

internal void
ShaderDelete(shader_program shader)
{
    glDeleteProgram(shader.id);
}


internal void
TextureBind(texture2d texture, i32 index)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    GLPrintErrors();
}

internal void
TextureDelete(texture2d texture)
{
    glDeleteTextures(1, &texture.id);
}

internal void
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

internal u32 
ShaderCreate(GLenum shaderType, const char *nullTerminatedShader)
{
    u32 shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &nullTerminatedShader, NULL);
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
    GLPrintErrors();

    return shader;
}

internal u32
ProgramCreate(u32 shaders[], u32 shaderCount)
{
    u32 shaderProgram;
    shaderProgram = glCreateProgram();

    for(u32 i = 0; i != shaderCount; i++)
    {
        glAttachShader(shaderProgram, shaders[i]);
    }

    glLinkProgram(shaderProgram);

    i32 status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        char log[ERROR_BUFFER_SIZE];
        glGetProgramInfoLog(shaderProgram, ERROR_BUFFER_SIZE - 1, NULL, log);

        LogError("%s", log);
    }

    for(u32 i = 0; i != shaderCount; i++)
        glDeleteShader(shaders[i]); 
    
    GLPrintErrors();

    return shaderProgram;
}


internal shader_program
ShaderCreate(const char *vertexShader, const char *fragmentShader)
{
    u32 vertexShaderId = ShaderCreate(GL_VERTEX_SHADER, vertexShader);       
    u32 fragmentShaderId = ShaderCreate(GL_FRAGMENT_SHADER, fragmentShader);       
    u32 shaders[2] = {vertexShaderId, fragmentShaderId};

    shader_program result;
    result.id = ProgramCreate(shaders, 2);

    return result;
}


// pathToResource relative to executable directory
internal texture2d 
TextureCreate(char *pathToResource)
{
    texture2d result = {};

    glGenTextures(1, &result.id);

    stbi_set_flip_vertically_on_load(true);  // FIXME: 
    u8 *resource = stbi_load("B:\\Programming\\Win32PlatformGithubCurrent\\source_code\\example_opengl_tetris\\bin\\awesomeface.png", 
                                &result.width, 
                                &result.height, 
                                &result.numberOfChannels, 0);

    glBindTexture(GL_TEXTURE_2D, result.id);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if(resource)
        {
            if(result.numberOfChannels == 4)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.width, result.height, 
                            0, GL_RGBA, GL_UNSIGNED_BYTE, resource);
                LogInfo("%s load %d", pathToResource, result.numberOfChannels);
            }
            else if(result.numberOfChannels == 3)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, result.width, result.height, 
                            0, GL_RGB, GL_UNSIGNED_BYTE, resource);
                LogInfo("%s load %d", pathToResource, result.numberOfChannels);
            }
            else 
            {
                LogError("Unsupported number of channels %s", pathToResource);
                Assert(0);
            }
            glGenerateMipmap(GL_TEXTURE_2D);
        } 
        else 
        {
            LogError("Failed to load resource %s", pathToResource);
            Assert(0);
        }
    }
    stbi_image_free(resource);
    GLPrintErrors();

    return result;
}

internal texture2d
TextureWhiteCreate()
{
    texture2d result = {};
    u32 whiteColor = 0xffffffff;

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whiteColor);
    }

    return result;
}



internal vertex_rectangle
VertexCreateRectangle(f32 x, f32 y, f32 z, f32 width, f32 height, f32 textureIndex, v4 color)
{
    vertex_rectangle result = {};

    f32 w = width + x;
    f32 h = height + y;

    result.vertices[0].position = {x, y, 0};
    result.vertices[0].color = color;
    result.vertices[0].textureCoordinate = {0.0f, 0.0f};
    result.vertices[0].textureIndex = textureIndex;

    result.vertices[1].position = {x, h, 0};
    result.vertices[1].color = color;
    result.vertices[1].textureCoordinate = {0.0f, 1.0f};
    result.vertices[1].textureIndex = textureIndex;

    result.vertices[2].position = {w, h, 0};
    result.vertices[2].color = color;
    result.vertices[2].textureCoordinate = {1.0f, 1.0f};
    result.vertices[2].textureIndex = textureIndex;

    result.vertices[3].position = {w, y, 0};
    result.vertices[3].color = color;
    result.vertices[3].textureCoordinate = {1.0f, 0.0f};
    result.vertices[3].textureIndex = textureIndex;

    return result;
}

internal vertex_rectangle
QuadTextured(v2 position, v2 size, f32 textureIndex)
{
    vertex_rectangle result = {};

    f32 x = position.x;
    f32 y = position.y;
    f32 z = 0;
    f32 w = size.width + x;
    f32 h = size.height + y;

    result.vertices[0].position = {x, y, z};
    result.vertices[0].color = {1, 1, 1, 1};
    result.vertices[0].textureCoordinate = {0.0f, 0.0f};
    result.vertices[0].textureIndex = textureIndex;

    result.vertices[1].position = {x, h, z};
    result.vertices[1].color = {1, 1, 1, 1};
    result.vertices[1].textureCoordinate = {0.0f, 1.0f};
    result.vertices[1].textureIndex = textureIndex;

    result.vertices[2].position = {w, h, z};
    result.vertices[2].color = {1, 1, 1, 1};
    result.vertices[2].textureCoordinate = {1.0f, 1.0f};
    result.vertices[2].textureIndex = textureIndex;

    result.vertices[3].position = {w, y, z};
    result.vertices[3].color = {1, 1, 1, 1};
    result.vertices[3].textureCoordinate = {1.0f, 0.0f};
    result.vertices[3].textureIndex = textureIndex;

    return result;
}

internal vertex_rectangle
QuadColored(v2 position, v2 size, v4 color)
{
    vertex_rectangle result = {};

    f32 x = position.x;
    f32 y = position.y;
    f32 z = 0;
    f32 w = size.width + x;
    f32 h = size.height + y;
    f32 textureIndex = 0;

    result.vertices[0].position = {x, y, z};
    result.vertices[0].color = color;
    result.vertices[0].textureCoordinate = {0.0f, 0.0f};
    result.vertices[0].textureIndex = textureIndex;

    result.vertices[1].position = {x, h, z};
    result.vertices[1].color = color;
    result.vertices[1].textureCoordinate = {0.0f, 1.0f};
    result.vertices[1].textureIndex = textureIndex;

    result.vertices[2].position = {w, h, z};
    result.vertices[2].color = color;
    result.vertices[2].textureCoordinate = {1.0f, 1.0f};
    result.vertices[2].textureIndex = textureIndex;

    result.vertices[3].position = {w, y, z};
    result.vertices[3].color = color;
    result.vertices[3].textureCoordinate = {1.0f, 0.0f};
    result.vertices[3].textureIndex = textureIndex;

    return result;
}

internal void
OpenGLRendererInitialize(opengl_renderer *renderer)
{
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &renderer->deviceTextureUnitCount);
    glGenBuffers(1, &renderer->vertexBufferIndex);
    glGenBuffers(1, &renderer->elementBufferIndex);
    glGenVertexArrays(1, &renderer->vertexArrayIndex);
    
    // NOTE: Setup the texture Array
    for(i32 i = 0; i < 32; i++) renderer->textureSlots[i] = i;
    
    // NOTE: Fill the element indices array with correct ordering of vertices
    // for every possible quad
    u32 offset = 0;
    for(u32 i = 0; i < maxQuadCount; i+=6)
    {
        renderer->elementArray[i] = 1 + offset;
        renderer->elementArray[i+1] = 0 + offset;
        renderer->elementArray[i+2] = 2 + offset;
        renderer->elementArray[i+3] = 2 + offset;
        renderer->elementArray[i+4] = 0 + offset;
        renderer->elementArray[i+5] = 3 + offset;
        
        offset += 4;
    }

    renderer->basicShader = ShaderCreate(vertexShader, fragmentShader);
    
    ShaderUse(renderer->basicShader);
    ShaderUniform(renderer->basicShader, "textures", renderer->textureSlots, renderer->deviceTextureUnitCount);
    
    renderer->textures[renderer->textureCount++] = TextureWhiteCreate();
    renderer->textures[renderer->textureCount++] = TextureCreate("/wall.jpg");
    renderer->textures[renderer->textureCount++] = TextureCreate("/awesomeface.png");
    
    TextureBind(renderer->textures[0], 0);
    TextureBind(renderer->textures[1], 1);
    TextureBind(renderer->textures[2], 2);
    
    glBindVertexArray(renderer->vertexArrayIndex);
    {
        // Vertex draw order buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->elementBufferIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(renderer->elementArray), renderer->elementArray, GL_STATIC_DRAW);
        
        // Buffer with our quads
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBufferIndex);
        glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->quadArray), 0, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, position));
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, color));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, textureCoordinate));
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)offsetof(vertex, textureIndex));
        
    }
    GLPrintErrors();
}

internal void
OpenGLRendererDestroy(opengl_renderer *renderer)
{
    // BUFFERS
    glDeleteBuffers(1, &renderer->vertexBufferIndex);
    glDeleteBuffers(1, &renderer->elementBufferIndex);
    glDeleteVertexArrays(1, &renderer->vertexArrayIndex);

    // SHADER
    ShaderDelete(renderer->basicShader);
    
    // TEXTRUES
    for(i32 i = 0; i < renderer->textureCount; i++)
    { 
        TextureDelete(renderer->textures[i]);
    }
    renderer->textureCount = 0;
}

internal void
PushQuad(opengl_renderer *opengl, vertex_rectangle quad)
{
    if(opengl->currentQuadCount < maxQuadCount)
    {
        opengl->quadArray[opengl->currentQuadCount++] = quad;
    }
    else
    {
        LogError("PushQuad max number of quads");
    }
}

internal void
BeginDrawing()
{
    glClearColor(0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    gl->currentQuadCount = 0;
}

internal void
EndDrawing()
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_rectangle) * gl->currentQuadCount, gl->quadArray);
    glDrawElements(GL_TRIANGLES, 6 * gl->currentQuadCount, GL_UNSIGNED_INT, 0);
    GLPrintErrors();
}