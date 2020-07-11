LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void Win32WindowSizeUpdate(HWND window);
f32 MonitorRefreshRateGet();
bool32 VSyncStateGet();
u64 ProcessorCyclesGet();
iv2 DrawAreaSizeGet();

void WindowSetSize(i32 width, i32 height);
void WindowSetPosition(i32 x, i32 y);

void WindowRefresh();
void WindowSetTransparency(u8 level);
void WindowNotAlwaysOnTop();
void WindowAlwaysOnTop();
void WindowDrawFrame(bool32 draw);
void Quit();