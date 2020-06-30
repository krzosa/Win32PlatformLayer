typedef struct window_dimension
{
    i32 width;
    i32 height;
} window_dimension;

typedef struct user_input
{
    bool8 up;
    bool8 down;
    bool8 left;
    bool8 right;
    bool8 reset;
} user_input;

typedef struct file
{
    void *contents;
    u32 size;
} file;

