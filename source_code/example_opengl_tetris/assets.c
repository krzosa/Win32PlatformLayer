struct FontSymbol
{
    // 8bit 0 - 255 per pixel
    i32 width;
    i32 height;
    i32 xOffset;
    i32 yOffset;
    V4 atlasBoundingBox;
    i32 xAdvance;
    u8 *symbolData;
};

struct Font
{
    u32 id;
    i32 width;
    i32 height;
    i32 symbolGapX;
    i32 symbolGapY;
    V2 symbolGapNormalized;
    FontSymbol symbols[95]; // 'A' - 33 to access the letter
};

Internal FontSymbol
FontLoadSymbol(MemoryArena *arena, stbtt_fontinfo *fontInfo, i32 codePoint, f32 scale) 
{
    FontSymbol result;
    
    u8 *bitmap = stbtt_GetCodepointBitmap(fontInfo, 0, scale, codePoint, 
                                          &result.width, &result.height, 
                                          &result.xOffset, &result.yOffset);
    
    stbtt_GetCodepointHMetrics(fontInfo, codePoint, &result.xAdvance, NULL);
    result.xAdvance = (i32)((f32)result.xAdvance * scale);
    
    // Flip Y 
    result.symbolData= (u8 *)ArenaPushArray(arena, u8, result.width * result.height);
    
    u8 *source = bitmap;
    u8 *destRow = result.symbolData  + (result.height - 1) * result.width;
    for(i32 y = 0; y < result.height; y++)
    {
        u8 *dest = destRow; 
        for(i32 x = 0; x < result.width; x++)
        {
            *dest++ = *source++;
        }
        destRow -= result.width;
    }
    stbtt_FreeBitmap(bitmap, 0);
    
    return result;
}

Internal Font
FontLoad(MemoryArena *arena, char *pathToResource)
{
    stbtt_fontinfo fontInfo;
    str8 *path = StringConcatChar(OS->exeDir, pathToResource);
    
    // TODO: Error checking
    u64 fileSize = OS->FileGetSize(path);
    u8 *ttfFile = (u8 *)ArenaPushSize(arena, fileSize);
    u64 bytesRead = OS->FileRead(path, ttfFile, fileSize);
    
    if(!stbtt_InitFont(&fontInfo, ttfFile, 0)) Assert(!"Error init font");
    float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, 48.0f);
    
    i32 ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    
    Font result = {};
    result.width = 512;
    result.height = 512;
    result.symbolGapX = 0;
    result.symbolGapY = 10;
    
    for(i32 i = 0; i < 94; i++)
    {
        result.symbols[i] = FontLoadSymbol(arena, &fontInfo, i + 33 , scaleFactor);
    }
    
    u8 *fontAtlas = (u8 *)ArenaPushArray(arena, u8, result.width * result.height);
    
    // NOTE(KKrzosa): Pack the symbols into a texture Atlas
    i32 plusWidth = 0;
    i32 plusHeight = 0;
    i32 highestHeightInRow = 0;
    for(i32 i = 0; i < 94; i++)
    {
        if(result.symbols[i].height > highestHeightInRow)
        {
            highestHeightInRow = result.symbols[i].height;
        }
        
        u8 *source = result.symbols[i].symbolData;
        u8 *destRow = fontAtlas + plusWidth + (plusHeight * (i32)result.width);
        
        V2 atlasOffsetNormalized = {
            (plusWidth / (f32)result.width),
            (plusHeight) / (f32)result.height
        };
        
        V2 normalizedSymbolSize = {
            (f32)result.symbols[i].width / (f32)result.width , 
            (f32)result.symbols[i].height / (f32)result.height
        };
        
        result.symbols[i].atlasBoundingBox = {
            atlasOffsetNormalized.x,
            atlasOffsetNormalized.y,
            atlasOffsetNormalized.x + normalizedSymbolSize.x,
            atlasOffsetNormalized.y + normalizedSymbolSize.y,
        };
        
        for(i32 y = 0; y < result.symbols[i].height; y++)
        {
            u8 *dest = destRow; 
            for(i32 x = 0; x < result.symbols[i].width; x++)
            {
                *dest++ = *source++;
            }
            destRow += (i32)result.width;
        }
        
        
        plusWidth += result.symbols[i].width + result.symbolGapX;
        
        // TODO(KKrzosa): i here overflows
        if(plusWidth + result.symbols[i+1].width + result.symbolGapX > result.width)
        {
            // precompute the height * width
            plusHeight += highestHeightInRow + result.symbolGapY;
            plusWidth = 0;
            highestHeightInRow = 0;
        }
        
        if(plusHeight + result.symbols[i+1].height > result.height)
        {
            Assert(!"Too many characters to fit in the atlas");
        }
    }
    
    
    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, result.width, result.height, 
                 0, GL_RED, GL_UNSIGNED_BYTE, fontAtlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLPrintErrors();
    
    
    StringFree(path);
    return result;
}

Internal Texture2D 
TextureCreate(char *pathToResource)
{
    Texture2D result = {};
    
    
    stbi_set_flip_vertically_on_load(true);  
    str8 *path = StringConcatChar(OS->exeDir, pathToResource);
    u8 *resource = stbi_load(path, 
                             &result.width, 
                             &result.height, 
                             &result.numberOfChannels, 0);
    StringFree(path);
    
    glGenTextures(1, &result.id);
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
    
    OpenGLRenderer *renderer = OpenGLRendererGet();
    Assert(renderer->textureCount < MAX_TEXTURE_COUNT);
    renderer->texturesOnGPU[renderer->textureCount++] = result.id;
    
    return result;
}
