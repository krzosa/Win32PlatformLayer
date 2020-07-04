typedef struct window_dimension
{
    i32 width;
    i32 height;
} window_dimension;

typedef struct user_input_controller
{
    f32 stickX;
    f32 stickY;

    u8 action1;
    u8 action2;
    u8 action3;
    u8 action4;

    u8 up;
    u8 down;
    u8 left;
    u8 right;


    u8 leftShoulder;
    u8 rightShoulder;
    
    u8 start;
    u8 select;
} user_input_controller;
