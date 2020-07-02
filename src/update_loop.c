#include "shared.h"

static OpenGLFunctions *gl;
#include "opengl.c"


void Initialize(application_memory *memory)
{
    gl = memory->gl;

    char *vertexShaderSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;"
        "void main()"
        "{"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "}\0";

    char *fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;"
        "void main(){"
            "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);}\0";

    u32 shaders[2];
    u32 shaderCount = 0;

    shaders[shaderCount++] = ShaderCreate(GL_VERTEX_SHADER, &vertexShaderSource);
    shaders[shaderCount++] = ShaderCreate(GL_FRAGMENT_SHADER, &fragmentShaderSource);

    u32 shaderProgram = ProgramCreate(shaders, shaderCount);

    u32 vertexBufferObject, vertexArrayObject;
    gl->GenVertexArrays(1, &vertexArrayObject);
    gl->GenBuffers(1, &vertexBufferObject); 

    f32 vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    }; 

    gl->BindVertexArray(vertexArrayObject);
    gl->BindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    gl->BufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    gl->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->EnableVertexAttribArray(0);
    gl->UseProgram(shaderProgram);
}
void Update(application_memory *memory)
{
    glClearColor(0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}
void HotReload(application_memory *memory)
{

}