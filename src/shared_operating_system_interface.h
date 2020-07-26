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
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
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

typedef struct time_data
{
    // READ ONLY
    // NOTE: count is a very granular unit of time
    // countsPerSecond says how many counts there are per second
    // I would advice to only use counts for debugging 
    i64 countsPerSecond;

    // TimeStamp taken at the program start
    // Cycles as in processor clock cycles
    u64 startAppCycles;
    u64 startAppCount;
    f32 startAppMilliseconds;

    // Length of the update, without the sleep 
    u64 updateCycles;
    i64 updateCount;
    f32 updateMilliseconds;

    // Length of the update, with sleep
    u64 frameCycles;
    i64 frameCount;
    f32 frameMilliseconds;
} time_data;

typedef struct operating_system_interface
{
    memory_storage pernamentStorage;
    memory_storage temporaryStorage;

    str8 *pathToExecutableDirectory;

    // NOTE: this buffer should be filled from the very beginning on every frame
    // by the ammount specified in requestedSamples, one sample should be 16bits
    // one frame (left right samples) should be 32 bits
    // this buffer is cleared on the os side on every frame
    void *audioBuffer; 
    u32 audioBufferSize;
    
    u32 requestedSamples; // number of samples to fill requested from the os
    u32 samplesPerSecond;
    // NOTE: // this value controls the audio latency, bigger number, bigger latency
    f32 audioLatencyMultiplier; 

    // NOTE: user input info, accessed through IsKeyDown etc.
    user_input userInput;
    // NOTE: update time, frame time, app start time
    time_data timeData;

    // NOTE: you can change fps by changing this value
    f32 targetFramesPerSecond;

    void   (*Quit)();
    void   (*Log)(char *text, ...);
    void   (*LogExtra)(char *prepend, char *text, ...);

    f32    (*TimeGetMilliseconds)();
    i64    (*TimeGetCounts)();
    u64    (*TimeGetProcessorCycles)();

    i64    (*FileGetSize)(char *filename); 
    i64    (*FileRead)(char *filename, void *memory, i64 bytesToRead); // returns bytes read or -1 when fail
    
    bool32 (*VSyncGetState)();
    bool32 (*VSyncSetState)(bool32 state);
    f32    (*MonitorGetRefreshRate)();
    
    iv2    (*WindowGetSize)();
    void   (*WindowSetTransparency)(u8 value);
    void   (*WindowAlwaysOnTop)();
    void   (*WindowNotAlwaysOnTop)();
    void   (*WindowSetSize)(i32 width, i32 height);
    void   (*WindowSetPosition)(i32 width, i32 height);
    void   (*WindowDrawBorder)(bool32 draw);
    
    void  *(*OpenGLLoadProcedures)(char *name);
} operating_system_interface;


