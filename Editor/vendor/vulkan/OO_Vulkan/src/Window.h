#pragma once

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif
//----------------------------------------------------------------------------------
// Process Window Message Callbacks
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Window
{

    enum WindowType
    {
        WINDOWS32,
        SDL2,


    };


    Window(uint32_t width =1024u, uint32_t height =720u, WindowType = WindowType::WINDOWS32);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    ~Window();
	void Init();

    HWND GetRawHandle()const;

    static bool PollEvents();

    uint32_t m_width;
    uint32_t m_height;
    WindowType m_type;
    void* rawHandle;

    bool windowShouldClose;

    static uint64_t SurfaceFormat;
    
};

