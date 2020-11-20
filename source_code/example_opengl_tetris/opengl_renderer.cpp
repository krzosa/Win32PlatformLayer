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

#include "opengl_utils.cpp"

Internal void
OpenGLRendererInit(MemoryArena *arena, OpenGLRenderer *renderer, 
                   f32 width, f32 height)
{
    OpenGLRendererAttach(renderer);
    renderer->width = width;
    renderer->height = height;
    
    glGenBuffers(1, &renderer->vertexBufferIndex);
    glGenBuffers(1, &renderer->elementBufferIndex);
    glGenVertexArrays(1, &renderer->vertexArrayIndex);
    
    
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &renderer->MAX_TEXTURE_IMAGE_UNITS);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &renderer->MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    
    LogInfo("MAX_TEXTURE_IMAGE_UNITS %d", renderer->MAX_TEXTURE_IMAGE_UNITS);
    LogInfo("MAX_COMBINED_TEXTURE_IMAGE_UNITS %d", renderer->MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    
    
    // NOTE: Setup the texture Array
    for(i32 i = 0; i < TEXTURE_NUMBERS_TO_MAKE_ON_GPU; i++) 
        renderer->textureSlots[i] = i;
    
    // NOTE: Fill the element indices array with correct ordering of vertices
    // for every possible quad
    u32 offset = 0;
    for(u32 i = 0; i < ELEMENT_COUNT; i+=6)
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
    ShaderUniform(renderer->basicShader, "textures", renderer->textureSlots, renderer->MAX_TEXTURE_IMAGE_UNITS);
    
    M4x4 projectionMatrix = OrtographicProjectionMatrix(0, width, 0, height, 0, 1);
    ShaderUniform(renderer->basicShader, "viewProjectionMatrix", projectionMatrix);
    
    
    glBindVertexArray(renderer->vertexArrayIndex);
    {
        // Vertex draw order buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                     renderer->elementBufferIndex);
        
        // TODO: should elements be STATIC_DRAW?
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * ELEMENT_COUNT,
                     renderer->elements, GL_STATIC_DRAW);
        
        // Buffer with our quads
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBufferIndex);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexRectangle) * MAX_QUAD_COUNT, 
                     0, GL_DYNAMIC_DRAW);
        
        
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
    
    renderer->whiteTexture = TextureWhiteCreate();
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, renderer->whiteTexture.id);
    
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
    
    // TEXTURES
    for(i32 i = 0; i < renderer->textureCount; i++)
    {
        glDeleteTextures(1, &renderer->texturesOnGPU[i]);
    }
}

Internal void
PushActiveDrawCall()
{
    OpenGLRenderer *gl = OpenGLRendererGet();
    
    // PushActiveActiveDrawCall on the draw array
    if(gl->activeDrawCall.type != DRAWCALLNull)
    {
        Assert(gl->drawCallCount < MAX_DRAW_CALLS);
        gl->drawCalls[gl->drawCallCount++] = gl->activeDrawCall;
        gl->activeDrawCall.type = DRAWCALLNull;
    }
}

Internal void
PushDrawCall(DrawCallType type, VertexRectangle rect, Texture2D texture = {})
{
    OpenGLRenderer *gl = OpenGLRendererGet();
    
    if(gl->activeDrawCall.type != type ||
       gl->activeDrawCall.texture.id != texture.id)
    {
        u32 offset = gl->activeDrawCall.dataOffset;
        u32 size = gl->activeDrawCall.dataSize;
        PushActiveDrawCall();
        
        gl->activeDrawCall.dataSize = 1;
        gl->activeDrawCall.dataOffset = offset + size;
        gl->activeDrawCall.type = type;
        gl->activeDrawCall.texture = texture;
    }
    else
    {
        gl->activeDrawCall.dataSize++;
    }
    
    Assert(gl->currentQuadCount < MAX_QUAD_COUNT);
    gl->quads[gl->currentQuadCount++] = rect;
}

Internal void
DrawBegin()
{
    OpenGLRenderer *gl = OpenGLRendererGet();
    
    gl->activeDrawCall = {};
    gl->currentQuadCount = 0;
    gl->drawCallCount = 0;
}

Internal void
DrawEnd()
{
    OpenGLRenderer *gl = OpenGLRendererGet();
    PushActiveDrawCall();
    
    glClearColor(0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    for(i32 i = 0; i < gl->drawCallCount; i++)
    {
        DrawCall *drawCall = &gl->drawCalls[i];
        switch(drawCall->type)
        {
            case DRAWCALLColor:
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gl->whiteTexture.id);
                
                glBufferSubData(GL_ARRAY_BUFFER,
                                0,
                                drawCall->dataSize * sizeof(VertexRectangle),
                                gl->quads + drawCall->dataOffset);
                
                glDrawElements(GL_TRIANGLES, 6 * drawCall->dataSize, 
                               GL_UNSIGNED_INT, 0);
                
                
                GLPrintErrors();
                break;
            }
            case DRAWCALLSprite:
            {
                glActiveTexture(GL_TEXTURE0 + 1);
                glBindTexture(GL_TEXTURE_2D, drawCall->texture.id);
                
                glBufferSubData(GL_ARRAY_BUFFER,
                                0,
                                drawCall->dataSize * sizeof(VertexRectangle),
                                gl->quads + drawCall->dataOffset);
                
                glDrawElements(GL_TRIANGLES, 6 * drawCall->dataSize, 
                               GL_UNSIGNED_INT, 0);
                
                GLPrintErrors();
                break;
            }
        }
    }
    
    GLPrintErrors();
}

Internal void
DrawRectangle(V4 rectangle, V4 color)
{
    VertexRectangle rect = RectangleColor(rectangle, color);
    PushDrawCall(DRAWCALLColor, rect);
}

Internal void
DrawSprite(V4 rectangle, Texture2D texture)
{
    VertexRectangle rect = RectangleTexture(rectangle, 1);
    PushDrawCall(DRAWCALLSprite, rect, texture);
}