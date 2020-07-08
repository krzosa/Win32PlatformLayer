LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
iv2 Win32GetWindowDimension(HWND window);
f32 MonitorRefreshRateGet();
bool32 VSyncStateGet();
u64 ProcessorCyclesGet();
iv2 DrawAreaSizeGet();
void Quit();