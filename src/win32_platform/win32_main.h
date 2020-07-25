LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void Win32WindowGetSizeUpdate();
f32 MonitorGetRefreshRate();
bool32 VSyncGetState();
u64 TimeGetProcessorCycles();
iv2 DrawAreaSizeGet();
iv2 Win32WindowDrawAreaGetSize();
iv2 Win32WindowGetSize();

void WindowSetSize(i32 width, i32 height);
void WindowDrawAreaSetSize(i32 width, i32 height);
void WindowSetPosition(i32 x, i32 y);

void WindowSetTransparency(u8 level);
void WindowDrawBorder(bool32 draw);
void WindowReloadAttributes();
void WindowNotAlwaysOnTop();
void WindowAlwaysOnTop();

void Quit();