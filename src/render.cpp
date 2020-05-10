#include <stdint.h>
#include "win32_main.h"

internal void RenderRectangle(win32_offscreen_buffer* buffer, int x, int y, int width, int height, int color)
{
    uint8_t *Row = (uint8_t *)buffer->memory; 
    Row = Row + (y * buffer->pitch);
    for(int Y = 0; Y < height; Y++)
    {
        uint32_t *Pixel = (uint32_t*)Row + x;
        for(int X = 0; X < width; X++)
        {
            uint8_t red = 0;
            uint8_t green = color ;
            uint8_t blue = 0;


            *Pixel = ((red << 16) | (green << 8) | (blue));
            Pixel = Pixel + 1;
        }
        Row = Row + buffer->pitch;
    }
}



internal void RenderWeirdGradient(win32_offscreen_buffer* buffer, int BlueOffset, int GreenOffset)
{    
    uint8_t *Row = (uint8_t *)buffer->memory;    
    for(int Y = 0; Y < buffer->height; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for(int X = 0; X < buffer->width; ++X)
        {
            uint8_t Blue = (X + BlueOffset);
            uint8_t Green = (Y + GreenOffset);
            
            *Pixel++ = ((Green << 16) | (Blue << 8) );
        }

        Row += buffer->pitch;
    }
}