LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void Win32WindowSizeUpdate();
f32 MonitorRefreshRateGet();
bool32 VSyncStateGet();
u64 ProcessorCyclesGet();
iv2 DrawAreaSizeGet();
iv2 Win32WindowDrawAreaSize();
iv2 Win32WindowSize();

void WindowWithBorderSetSize(i32 width, i32 height);
void WindowSetSize(i32 width, i32 height);
void WindowSetPosition(i32 x, i32 y);

void WindowRefresh();
void WindowSetTransparency(u8 level);
void WindowNotAlwaysOnTop();
void WindowAlwaysOnTop();
void WindowDrawBorder(bool32 draw);
void Quit();