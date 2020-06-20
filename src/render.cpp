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
DrawLine(win32_offscreen_buffer* buffer, v2 point1, v2 point2, v4 color)
{
    i32 *pixel;
    for(f32 t = 0.; t < 1.; t += 0.001)
    {
        i32 x = point1.x + t * (point2.x - point1.x);
        i32 y = point1.y + t * (point2.y - point1.y);
        pixel = PixelGet(buffer, x, y);
        PixelColor(pixel, color);
    }
}