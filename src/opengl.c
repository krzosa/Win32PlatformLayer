// TODO: move maybe to dynamically allocated string, but probably I need a custom allocator first

internal u32 
CreateShader(GLenum shaderType, char **nullTerminatedShaderFile)
{
    u32 shader = gl.CreateShader(shaderType);
    gl.ShaderSource(shader, 1, nullTerminatedShaderFile, NULL);
    gl.CompileShader(shader);

    char log[2048];
    i32 status;
    gl.GetShaderiv(shaderType, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        gl.GetShaderInfoLog(shader, 512, NULL, log);
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

    i32 status;
    char log[2048];
    gl.LinkProgram(shaderProgram);
    gl.GetShaderiv(shaderProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        gl.GetProgramInfoLog(shaderProgram, 512, NULL, log);
        LogError("SHADER VERTEX LINKING %s", log);
    }

    gl.DeleteShader(vertexShader);
    gl.DeleteShader(fragmentShader); 
}