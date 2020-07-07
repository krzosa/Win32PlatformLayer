typedef struct memory_storage
{
    void *memory;
    u64 maxSize;
    u64 allocatedSize;
    u64 highestAllocatedSize;
} memory_storage;

typedef struct file_data
{
    void *contents;
    u32 size;
} file_data;

typedef enum keyboard_keys
{
    KEY_W,
    KEY_S,
    KEY_A,
    KEY_D,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F12,
    KEY_ESC,

    KEY_COUNT,
} keyboard_keys;

typedef enum controller_buttons
{
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,

    BUTTON_DPAD_UP,
    BUTTON_DPAD_DOWN,
    BUTTON_DPAD_LEFT,
    BUTTON_DPAD_RIGHT,

    BUTTON_LEFT_SHOULDER,
    BUTTON_RIGHT_SHOULDER,

    BUTTON_START,
    BUTTON_SELECT,

    BUTTON_COUNT,
} controller_buttons;

typedef struct user_input_controller
{
    f32 leftStickX;
    f32 leftStickY;

    f32 rightStickX;
    f32 rightStickY;
    
    bool8 connected;
    bool8 currentButtonState[BUTTON_COUNT];
    bool8 previousButtonState[BUTTON_COUNT];
} user_input_controller;

typedef struct user_input_keyboard
{
    bool8 currentKeyState[KEY_COUNT];
    bool8 previousKeyState[KEY_COUNT];
} user_input_keyboard;

typedef struct user_input_mouse
{
    i32 mousePosX;
    i32 mousePosY;

    bool8 left;
    bool8 right;
    bool8 middle;
} user_input_mouse;

typedef struct user_input
{
    user_input_controller controller[4];
    user_input_keyboard keyboard;
    user_input_mouse mouse;
} user_input;

typedef struct operating_system_interface
{
    memory_storage pernamentStorage;
    memory_storage temporaryStorage;

    void *audioBuffer; 
    u32 audioBufferSize;
    u32 requestedSamples; // number of samples to fill requested from the os
    u32 samplesPerSecond;

    user_input userInput;

    f32 targetMsPerFrame;
    f32 monitorRefreshRate;
    iv2 windowSize;
    bool32 vsync;

    void   (*log)(char *text, ...);
    void   (*logExtra)(char *prepend, char *text, ...);
    f32    (*timeCurrentGet)();
    void  *(*OpenGLFunctionLoad)(char *name);
    bool32 (*VSyncSet)(bool32 state);
    void   (*RefreshScreen)();
} operating_system_interface;


