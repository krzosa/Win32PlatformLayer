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