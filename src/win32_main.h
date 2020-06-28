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

