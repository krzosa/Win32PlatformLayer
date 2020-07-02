// TODO: move maybe to dynamically allocated string, but probably I need a custom allocator first
#define ERROR_BUFFER_SIZE 1028

internal u32 
ShaderCreate(GLenum shaderType, char **nullTerminatedShaderFile)
{
    u32 shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, nullTerminatedShaderFile, NULL);
    glCompileShader(shader);

    i32 status;
    glGetShaderiv(shaderType, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        char log[ERROR_BUFFER_SIZE];
        glGetShaderInfoLog(shader, ERROR_BUFFER_SIZE - 1, NULL, log);

        char *strShaderType = NULL;
        switch(shaderType)
        {
            case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
            case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
            case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
        }
        // LogError("%s Shader compilation %s", shaderType, log);
    }

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

        // LogError("Create program %s", log);
    }

    for(u32 i = 0; i != shaderCount; i++)
        glDeleteShader(shaders[i]); 

    return shaderProgram;
}