// TODO: move maybe to dynamically allocated string, but probably I need a custom allocator first
#define ERROR_BUFFER_SIZE 2048

internal u32 
CreateShader(GLenum shaderType, char **nullTerminatedShaderFile)
{
    u32 shader = gl.CreateShader(shaderType);
    gl.ShaderSource(shader, 1, nullTerminatedShaderFile, NULL);
    gl.CompileShader(shader);

    i32 status;
    gl.GetShaderiv(shaderType, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        char log[ERROR_BUFFER_SIZE];
        gl.GetShaderInfoLog(shader, ERROR_BUFFER_SIZE - 1, NULL, log);

        char *strShaderType = NULL;
        switch(shaderType)
        {
            case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
            case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
            case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
        }
        LogError("%s Shader compilation %s", shaderType, log);
    }

    return shader;
}

internal u32
CreateProgram(u32 shaders[], u32 shaderCount)
{
    u32 shaderProgram = gl.CreateProgram();

    for(u32 i = 0; i != shaderCount; i++)
        gl.AttachShader(shaderProgram, shaders[i]);

    gl.LinkProgram(shaderProgram);

    i32 status;
    gl.GetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        char log[ERROR_BUFFER_SIZE];
        gl.GetProgramInfoLog(shaderProgram, ERROR_BUFFER_SIZE - 1, NULL, log);

        LogError("Create program %s", log);
    }

    for(u32 i = 0; i != shaderCount; i++)
        gl.DeleteShader(shaders[i]); 

    return shaderProgram;
}