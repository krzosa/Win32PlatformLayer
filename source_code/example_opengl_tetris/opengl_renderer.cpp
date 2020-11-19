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

struct OpenGLRenderer
{
    ShaderProgram basicShader;
    
    u32 maxQuadCount;
    u32 maxVertexCount;
    u32 elementsPerQuad;
    u32 maxTextureCount;
    
    u32 vertexBufferIndex; 
    u32 vertexArrayIndex;
    u32 elementBufferIndex;
    
    i32 deviceTextureUnitCount;
    i32 textureSlots[32];
    
    Texture2D *textures;
    u32 textureCount;
    
    u32 *elements; // array 
    u32 elementCount;
    VertexRectangle *quads; //array
    u32 currentQuadCount;
};

// GLOBAL POINTER
// Shouldn't be accessed directly because it can lead to
// weird crashes when the pointer is null and you don't realize
// Get and Attach functions add Assertions to check for null pointers
Global OpenGLRenderer *privateOpenGLRenderer;

Internal void
OpenGLRendererAttach(OpenGLRenderer *gl)
{
    Assert(gl != 0);
    privateOpenGLRenderer = gl;
}

Internal OpenGLRenderer *
OpenGLRendererGet()
{
    Assert(privateOpenGLRenderer != 0);
    return privateOpenGLRenderer;
}

Internal i32
ShaderGetUniformLocation(ShaderProgram shader, char *uniformName)
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

Internal void 
ShaderUniform(ShaderProgram shader, char *uniformName, i32 value)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniform1i(uniformLocation, value);
}

Internal void 
ShaderUniform(ShaderProgram shader, char *uniformName, M4x4 matrix)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &matrix.e[0][0]);
}

Internal void 
ShaderUniform(ShaderProgram shader, char *uniformName, i32 value[], i32 count)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniform1iv(uniformLocation, count, value);
}

Internal void 
ShaderUniform(ShaderProgram shader, char *uniformName, V4 vector)
{
    i32 uniformLocation = ShaderGetUniformLocation(shader, uniformName);
    glUniform4f(uniformLocation, vector.x, vector.y, vector.z, vector.w);
}

Internal void
ShaderUse(ShaderProgram shader)
{
    glUseProgram(shader.id);
}

Internal void
ShaderDelete(ShaderProgram shader)
{
    glDeleteProgram(shader.id);
}


Internal void
TextureBind(Texture2D texture, i32 index)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    GLPrintErrors();
}

Internal void
TextureBind(u32 indexTexture, i32 indexOpenGLSlot)
{
    glActiveTexture(GL_TEXTURE0 + indexOpenGLSlot);
    OpenGLRenderer *renderer = OpenGLRendererGet();
    glBindTexture(GL_TEXTURE_2D, renderer->textures[indexTexture].id);
    GLPrintErrors();
}

Internal void
TextureDelete(Texture2D texture)
{
    glDeleteTextures(1, &texture.id);
}

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

Internal u32 
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

Internal u32
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


Internal ShaderProgram
ShaderCreate(const char *vertexShader, const char *fragmentShader)
{
    u32 vertexShaderId = ShaderCreate(GL_VERTEX_SHADER, vertexShader);       
    u32 fragmentShaderId = ShaderCreate(GL_FRAGMENT_SHADER, fragmentShader);       
    u32 shaders[2] = {vertexShaderId, fragmentShaderId};
    
    ShaderProgram result;
    result.id = ProgramCreate(shaders, 2);
    
    return result;
}

Internal u32
PushTexture(Texture2D texture)
{
    OpenGLRenderer *renderer = OpenGLRendererGet();
    
    Assert(renderer->maxTextureCount > renderer->textureCount);
    u32 index = renderer->textureCount++;
    renderer->textures[index] = texture;
    
    return index;
}

// @arguments: pathToResource relative to executable directory
// @return:    index to the texture array
Internal u32 
TextureCreate(char *pathToResource)
{
    Texture2D texture = {};
    
    glGenTextures(1, &texture.id);
    
    stbi_set_flip_vertically_on_load(true);  
    str8 *path = StringConcatChar(OS->exeDir, pathToResource);
    u8 *resource = stbi_load(path, 
                             &texture.width, 
                             &texture.height, 
                             &texture.numberOfChannels, 0);
    StringFree(path);
    
    glBindTexture(GL_TEXTURE_2D, texture.id);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        if(resource)
        {
            if(texture.numberOfChannels == 4)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 
                             0, GL_RGBA, GL_UNSIGNED_BYTE, resource);
                LogInfo("%s load %d", pathToResource, texture.numberOfChannels);
            }
            else if(texture.numberOfChannels == 3)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 
                             0, GL_RGB, GL_UNSIGNED_BYTE, resource);
                LogInfo("%s load %d", pathToResource, texture.numberOfChannels);
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
    
    u32 result = PushTexture(texture);
    
    return result;
}

// @return: index to the texture array
Internal u32
TextureWhiteCreate()
{
    Texture2D texture = {};
    u32 whiteColor = 0xffffffff;
    
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whiteColor);
    }
    u32 result = PushTexture(texture);
    
    return result;
}


Internal VertexRectangle
VertexCreateRectangle(f32 x, f32 y, f32 z, f32 width, f32 height, f32 textureIndex, V4 color)
{
    VertexRectangle result = {};
    
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

Internal VertexRectangle
QuadTextured(V4 rectangle, f32 textureIndex)
{
    VertexRectangle result = {};
    
    f32 x = rectangle.x;
    f32 y = rectangle.y;
    f32 z = 0;
    f32 w = rectangle.width + x;
    f32 h = rectangle.height + y;
    
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

Internal VertexRectangle
QuadColored(V4 rectangle, V4 color)
{
    VertexRectangle result = {};
    
    f32 x = rectangle.x;
    f32 y = rectangle.y;
    f32 z = 0;
    f32 w = rectangle.width + x;
    f32 h = rectangle.height + y;
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

Internal void
OpenGLRendererInitialize(MemoryArena *arena, OpenGLRenderer *renderer)
{
    renderer->maxQuadCount = 10000;
    renderer->maxVertexCount = renderer->maxQuadCount * 4; 
    renderer->elementsPerQuad = 6;
    renderer->maxTextureCount = 128;
    
    renderer->elementCount = renderer->elementsPerQuad * renderer->maxQuadCount;
    renderer->textures = ArenaPushArray(arena, Texture2D, renderer->maxTextureCount);
    renderer->elements = ArenaPushArray(arena, u32, renderer->elementCount);
    renderer->quads = ArenaPushArray(arena, VertexRectangle, renderer->maxQuadCount);
    OpenGLRendererAttach(renderer);
    
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &renderer->deviceTextureUnitCount);
    glGenBuffers(1, &renderer->vertexBufferIndex);
    glGenBuffers(1, &renderer->elementBufferIndex);
    glGenVertexArrays(1, &renderer->vertexArrayIndex);
    
    // NOTE: Setup the texture Array
    for(i32 i = 0; i < 32; i++) renderer->textureSlots[i] = i;
    
    // NOTE: Fill the element indices array with correct ordering of vertices
    // for every possible quad
    u32 offset = 0;
    for(u32 i = 0; i < renderer->elementCount; i+=6)
    {
        renderer->elements[i] = 1 + offset;
        renderer->elements[i+1] = 0 + offset;
        renderer->elements[i+2] = 2 + offset;
        renderer->elements[i+3] = 2 + offset;
        renderer->elements[i+4] = 0 + offset;
        renderer->elements[i+5] = 3 + offset;
        
        offset += 4;
        // if(i > 100) Assert(offset > 100); // check for overflow
    }
    
    renderer->basicShader = ShaderCreate(vertexShader, fragmentShader);
    
    ShaderUse(renderer->basicShader);
    ShaderUniform(renderer->basicShader, "textures", renderer->textureSlots, renderer->deviceTextureUnitCount);
    
    u32 whiteTexture = TextureWhiteCreate();
    u32 wall = TextureCreate("/data/wall.jpg");
    u32 face = TextureCreate("/data/awesomeface.png");
    
    TextureBind(whiteTexture, 0);
    TextureBind(wall, 1);
    TextureBind(face, 2);
    
    glBindVertexArray(renderer->vertexArrayIndex);
    {
        // Vertex draw order buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                     renderer->elementBufferIndex);
        
        // TODO: should elements be STATIC_DRAW?
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(u32) * renderer->elementCount,
                     renderer->elements, GL_STATIC_DRAW);
        
        // Buffer with our quads
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBufferIndex);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(VertexRectangle) * renderer->maxQuadCount, 
                     0,
                     GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, position));
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, color));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, textureCoordinate));
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, textureIndex));
    }
    GLPrintErrors();
}

Internal void
OpenGLRendererDestroy()
{
    OpenGLRenderer *renderer = OpenGLRendererGet();
    
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

Internal void
PushQuad(VertexRectangle quad)
{
    OpenGLRenderer *renderer = OpenGLRendererGet();
    
    if(renderer->currentQuadCount < renderer->maxQuadCount)
    {
        renderer->quads[renderer->currentQuadCount++] = quad;
    }
    else
    {
        LogError("PushQuad max number of quads");
    }
}

Internal void
DrawBegin()
{
    glClearColor(0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    OpenGLRenderer *gl = OpenGLRendererGet();
    gl->currentQuadCount = 0;
}

Internal void
DrawEnd()
{
    OpenGLRenderer *gl = OpenGLRendererGet();
    GLPrintErrors();
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(VertexRectangle) * gl->currentQuadCount,
                    gl->quads);
    GLPrintErrors();
    
    glDrawElements(GL_TRIANGLES, 6 * gl->currentQuadCount, 
                   GL_UNSIGNED_INT, 0);
    
    GLPrintErrors();
}

Internal void
DrawRectangle(V4 rectangle, V4 color)
{
    VertexRectangle rect = QuadColored(rectangle, color);
    PushQuad(rect);
}

Internal void
DrawSprite(V4 rectangle, i32 texture)
{
    VertexRectangle rect = QuadTextured(rectangle, (f32)texture);
    PushQuad(rect);
}