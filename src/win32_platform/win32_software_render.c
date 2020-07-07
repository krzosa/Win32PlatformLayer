struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void *memory;
    i32 width;
    i32 height;
    i32 pitch;
    // Note: Pixels are always 32bits wide
};

iv2 
Win32GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    iv2 windowDimension;
    // get size of the window, without the border
    GetClientRect(Window, &ClientRect);
    windowDimension.width = ClientRect.right - ClientRect.left;
    windowDimension.height = ClientRect.bottom - ClientRect.top;
    return windowDimension;
}

internal void 
Win32ResizeDIBSection(win32_offscreen_buffer* buffer, i32 width, i32 height)
{
    // if we dont free before allocating, memory will leak
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    i32 bytesPerPixel = 4;
    buffer->width = width;
    buffer->height = height;
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB; //uncompressed RGB

    
    i32 bitmapMemorySize = bytesPerPixel * (buffer->width * buffer->height);
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytesPerPixel;
}

internal void 
Win32DrawBufferToScreen(HDC DeviceContext, i32 windowWidth, i32 windowHeight, win32_offscreen_buffer* buffer)
{
    // The StretchDIBits function copies the color data for a rectangle of pixels in a DIB, 
    // to the specified destination rectangle. 
    // If the destination rectangle is larger than the source rectangle, 
    // this function stretches the rows and columns of color data to fit the destination rectangle. 
    // If the destination rectangle is smaller than the source rectangle, 
    // this function compresses the rows and columns by using the specified raster operation.
    StretchDIBits(
        DeviceContext,
        0, 0, windowWidth, windowHeight,
        0, 0, buffer->width, buffer->height,
        buffer->memory,
        &buffer->info,
        DIB_RGB_COLORS, SRCCOPY);
}

internal void 
Swap(f32 *a, f32 *b)
{
    f32 temp = *a;
    *a = *b;
    *b = temp;
}

internal void 
Swap(i32 *a, i32 *b)
{
    i32 temp = *a;
    *a = *b;
    *b = temp;
}

internal f32
Clamp(f32 val, f32 min, f32 max)
{
    if(val < min) return min;
    else if(val > max) return max;
    return val;
}

internal i32
Clamp(i32 val, i32 min, i32 max)
{
    if(val < min) return min;
    else if(val > max) return max;
    return val;
}

internal i32 *
PixelGet(i32 x, i32 y)
{
    i8 *row = (i8 *)GlobalBackbuffer.memory;
    row += (y * GlobalBackbuffer.pitch);

    i32 *pixel = (i32 *)row;
    pixel += x;

    return pixel;
}

internal void 
PixelColor(i32 *pixel, v4 color)
{
    *pixel = ((color.r << 16) | (color.g << 8) | (color.b));
}

internal void
PixelSet(i32 x, i32 y, v4 color)
{
    if(x <= 0 || y <= 0) return;
    if(x >= GlobalBackbuffer.width - 1 || y >= GlobalBackbuffer.height) return;

    i8 *pixelQuarter = (i8 *)GlobalBackbuffer.memory;

    pixelQuarter += (y * GlobalBackbuffer.pitch);
    pixelQuarter += x * 4;

    pixelQuarter[0] = color.b;
    pixelQuarter[1] = color.g;
    pixelQuarter[2] = color.r;
}

internal void 
DrawRectangle(i32 x, i32 y, i32 width, i32 height, i32 color)
{
    u8 *Row = (u8 *)GlobalBackbuffer.memory; 
    Row = Row + (y * GlobalBackbuffer.pitch);
    for(i32 Y = 0; Y < height; Y++)
    {
        i32 *Pixel = (i32*)Row + x;
        for(i32 X = 0; X < width; X++)
        {
            u8 red = 0;
            u8 green = color ;
            u8 blue = 0;


            *Pixel = ((red << 16) | (green << 8) | (blue));
            Pixel = Pixel + 1;
        }
        Row = Row + GlobalBackbuffer.pitch;
    }
}


internal void 
DrawGradient(i32 BlueOffset, i32 GreenOffset)
{    
    u8 *Row = (u8 *)GlobalBackbuffer.memory;    
    for(i32 Y = 0; Y < GlobalBackbuffer.height; ++Y)
    {
        i32 *Pixel = (i32 *)Row;
        for(i32 X = 0; X < GlobalBackbuffer.width; ++X)
        {
            u8 Blue = (X + BlueOffset);
            u8 Green = (Y + GreenOffset);
            
            *Pixel++ = ((Green << 16) | (Blue << 8) );
        }

        Row += GlobalBackbuffer.pitch;
    }
}

internal void
DrawLine(i32 startPosX, i32 startPosY, i32 endPosX, i32 endPosY, v4 color)
{
    bool steep = false;
    if(abs(startPosX - endPosX) < abs(startPosY - endPosY))
    {
        Swap(&startPosX, &startPosY);
        Swap(&endPosX, &endPosY);
        steep = true;
    }
    if(startPosX > endPosX)
    {
        Swap(&startPosX, &endPosX);
        Swap(&startPosY, &endPosY);
    }

    i32 lineLengthX = endPosX - startPosX;
    i32 y = startPosY;
    if(steep)
        for(i32 x = startPosX; x <= endPosX; x++)
        {
            // NOTE: distance from the baseX to currently drawn x 
            // divided by the entire distance from start to end
            // calculating the pixel number basically 
            f32 t = (x - startPosX) / (f32)(endPosX - startPosX);
            // NOTE: Calculating y based on the pixel number
            i32 y = startPosY * (1. - t) + endPosY * t;
            PixelSet(y, x, color);
        }
    else
        for(i32 x = startPosX; x <= endPosX; x++)
        {
            f32 t = (x - startPosX) / (f32)(endPosX - startPosX);
            // NOTE: Calculating y based on the pixel number
            i32 y = startPosY * (1. - t) + endPosY * t;
            PixelSet(x, y, color);
        }
}