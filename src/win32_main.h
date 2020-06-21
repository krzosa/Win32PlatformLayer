struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void *memory;
    i32 width;
    i32 height;
    i32 pitch;
    // Note: Pixels are always 32bits wide
};

struct window_dimension
{
    i32 width;
    i32 height;
};

struct user_input
{
    bool up;
    bool down;
    bool left;
    bool right;
    bool reset;
};

struct file
{
    void *contents;
    u32 size;
};

struct win32_sound_output
{
    i32 samplesPerSecond;
    i32 toneHz;
    i16 toneVolume;
    i32 runningSampleIndex;
    i32 wavePeriod;
    i32 bytesPerSample;
    i32 secondaryBufferSize;
    f32 tSine;
    i32 latencySampleCount;
};
