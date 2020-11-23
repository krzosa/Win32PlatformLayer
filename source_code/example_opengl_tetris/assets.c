struct FontSymbol
{
    // 8bit 0 - 255 per pixel
    i32 width;
    i32 height;
    i32 xOffset;
    i32 yOffset;
    V4 atlasBoundingBox;
    V4 stbBoundingBox;
    i32 xAdvance;
    u8 *symbolData;
};

struct Font
{
    u32 id;
    i32 width;
    i32 height;
    i32 ascent;
    i32 descent;
    i32 linegap;
    V2 symbolGapNormalized;
    FontSymbol symbols[96]; // 'A' - 32 to access the letter
};

Internal FontSymbol
FontLoadSymbol(MemoryArena *arena, stbtt_fontinfo *fontInfo, i32 codePoint, f32 scale, i32 ascent) 
{
    FontSymbol result;
    
    u8 *bitmap = stbtt_GetCodepointBitmap(fontInfo, 0, scale, codePoint, 
                                          &result.width, &result.height, 
                                          &result.xOffset, &result.yOffset);
    
    stbtt_GetCodepointHMetrics(fontInfo, codePoint, &result.xAdvance, NULL);
    
    i32 x0, y0, x1, y1;
    stbtt_GetCodepointBox(fontInfo, codePoint, &x0, &y0, &x1, &y1);
    
    result.stbBoundingBox = {
        ((f32)x0 * scale),
        ((f32)y0 * scale),
        ((f32)x1 * scale),
        ((f32)y1 * scale),
    };
    
    result.xAdvance = (i32)((f32)result.xAdvance * scale);
    
    //
    // Flip Y 
    //

    result.stbBoundingBox.y1 *= -1;
    result.stbBoundingBox.y2 *= -1;
    result.yOffset *= -1;
    
    result.symbolData = (u8 *)ArenaPushArray(arena, u8, result.width * result.height);
    
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
    u64 fileSize = OS->FileGetSize(path); Assert(fileSize != 0);
    u8 *ttfFile = (u8 *)ArenaPushSize(arena, fileSize);
    u64 bytesRead = OS->FileRead(path, ttfFile, fileSize);
    
    if(!stbtt_InitFont(&fontInfo, ttfFile, 0)) Assert(!"Error init font");
    float scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, 48.0f);
    
    Font result = {};
    stbtt_GetFontVMetrics(&fontInfo, &result.ascent, &result.descent, &result.linegap);
    result.ascent = (f32)result.ascent * scaleFactor;
    result.descent = (f32)result.descent * scaleFactor;
    result.linegap = (f32)result.linegap * scaleFactor;
    result.width = 512;
    result.height = 512;

    // Load symbols

    for(i32 i = 0; i < 94; i++)
    {
        result.symbols[i] = FontLoadSymbol(arena, &fontInfo, i + 32 , scaleFactor, result.ascent);
    }
    
    //
    // Pack the symbols into a texture Atlas
    //

    i32 plusWidth = 0;
    i32 plusHeight = 0;
    i32 symbolGapX = 10;
    i32 symbolGapY = 10;
    i32 highestHeightInRow = 0;
    u8 *fontAtlas = (u8 *)ArenaPushArray(arena, u8, result.width * result.height);
    for(i32 i = 0; i < 94; i++)
    {
        if(result.symbols[i].height > highestHeightInRow)
        {
            highestHeightInRow = result.symbols[i].height;
        }
        
        ///
        /// Calculate normalized Atlas bounding box
        ///         for texture coordinates
        ///
        
        V2 atlasOffsetNormalized = {
            (f32)(plusWidth / (f32)result.width),
            (f32)(plusHeight) / (f32)result.height,
        };
        
        V2 normalizedSymbolSize = {
            (f32)result.symbols[i].width / (f32)result.width , 
            (f32)(result.symbols[i].height) / (f32)result.height
        };
        
        result.symbols[i].atlasBoundingBox = {
            atlasOffsetNormalized.x,
            atlasOffsetNormalized.y,
            atlasOffsetNormalized.x + normalizedSymbolSize.x,
            atlasOffsetNormalized.y + normalizedSymbolSize.y,
        };

        //
        // Pack the symbols into atlas
        //
        
        u8 *source = result.symbols[i].symbolData;
        u8 *destRow = fontAtlas + plusWidth + (plusHeight * (i32)result.width);
        
        for(i32 y = 0; y < result.symbols[i].height; y++)
        {
            u8 *dest = destRow; 
            for(i32 x = 0; x < result.symbols[i].width; x++)
            {
                *dest++ = *source++;
            }
            destRow += (i32)result.width;
        }

        //
        // Measure the symbol packing, by how much to advance the pointer
        //

        plusWidth += result.symbols[i].width + symbolGapX;
        
        // TODO(KKrzosa): i here overflows
        if(plusWidth + result.symbols[i+1].width + symbolGapX > result.width)
        {
            // precompute the height * width
            plusHeight += highestHeightInRow + symbolGapY;
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
