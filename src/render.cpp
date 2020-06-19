#include "win32_main.h"

internal void RenderRectangle(win32_offscreen_buffer* buffer, int x, int y, int width, int height, int color)
{
    u8 *Row = (u8 *)buffer->memory; 
    Row = Row + (y * buffer->pitch);
    for(int Y = 0; Y < height; Y++)
    {
        i32 *Pixel = (i32*)Row + x;
        for(int X = 0; X < width; X++)
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



internal void RenderWeirdGradient(win32_offscreen_buffer* buffer, i32 BlueOffset, i32 GreenOffset)
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