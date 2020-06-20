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
PixelGet(win32_offscreen_buffer* buffer, i32 x, i32 y)
{
    i8 *row = (i8 *)buffer->memory;
    row += (y * buffer->pitch);

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
PixelSet(win32_offscreen_buffer* buffer, i32 x, i32 y, v4 color)
{
    i8 *row = (i8 *)buffer->memory;
    row += (y * buffer->pitch);

    i32 *pixel = (i32 *)row;
    pixel += x;

    *pixel = ((color.r << 16) | (color.g << 8) | (color.b));
}

internal void 
DrawRectangle(win32_offscreen_buffer* buffer, i32 x, i32 y, i32 width, i32 height, i32 color)
{
    u8 *Row = (u8 *)buffer->memory; 
    Row = Row + (y * buffer->pitch);
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
        Row = Row + buffer->pitch;
    }
}


internal void 
DrawGradient(win32_offscreen_buffer* buffer, i32 BlueOffset, i32 GreenOffset)
{    
    u8 *Row = (u8 *)buffer->memory;    
    for(i32 Y = 0; Y < buffer->height; ++Y)
    {
        i32 *Pixel = (i32 *)Row;
        for(i32 X = 0; X < buffer->width; ++X)
        {
            u8 Blue = (X + BlueOffset);
            u8 Green = (Y + GreenOffset);
            
            *Pixel++ = ((Green << 16) | (Blue << 8) );
        }

        Row += buffer->pitch;
    }
}

internal void
DrawLineFirst(win32_offscreen_buffer* buffer, v2 point1, v2 point2, v4 color)
{
    for(f32 t = 0.; t < 1.; t += 0.01)
    {
        i32 x = point1.x + t * (point2.x - point1.x);
        i32 y = point1.y + t * (point2.y - point1.y);
        PixelSet(buffer, x, y, color);
    }
}

internal void
DrawLineSecond(win32_offscreen_buffer* buffer, v2 point1, v2 point2, v4 color)
{
    for(i32 x = point1.x; x <= point2.x; x++)
    {
        float t = (x - point1.x) / (float)(point2.x - point1.x);
        i32 y = point1.y * (1. - t) + point2.y * t;
        PixelSet(buffer, x, y, color);
    }
}

internal void
DrawLineThird(win32_offscreen_buffer* buffer, v2 point1, v2 point2, v4 color)
{
    point1.y = Clamp(point1.y, 50., buffer->height - 50.);
    point2.y = Clamp(point2.y, 50., buffer->height - 50.);

    bool steep = false;
    if(fabsf(point1.x - point2.x) < fabsf(point1.y - point2.y))
    {
        Swap(&point1.x, &point1.y);
        Swap(&point2.x, &point2.y);
        steep = true;
    }
    if(point1.x > point2.x)
    {
        Swap(&point1.x, &point2.x);
        Swap(&point1.y, &point2.y);
    }
    for(i32 x = point1.x; x <= point2.x; x++)
    {
        // NOTE: distance from the baseX to currently drawn x 
        // divided by the entire distance from start to end
        // calculating the pixel number basically 
        f32 t = (x - point1.x) / (f32)(point2.x - point1.x);
        // NOTE: Calculating y based on the pixel number
        i32 y = point1.y * (1. - t) + point2.y * t;
        if(steep)
            PixelSet(buffer, y, x, color);
        else
            PixelSet(buffer, x, y, color);

    }
}

internal void
DrawLineFinal(win32_offscreen_buffer* buffer, i32 startPosX, i32 startPosY, i32 endPosX, i32 endPosY, v4 color)
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
            PixelSet(buffer, y, x, color);
        }
    else
        for(i32 x = startPosX; x <= endPosX; x++)
        {
            f32 t = (x - startPosX) / (f32)(endPosX - startPosX);
            // NOTE: Calculating y based on the pixel number
            i32 y = startPosY * (1. - t) + endPosY * t;
            PixelSet(buffer, x, y, color);
        }
}