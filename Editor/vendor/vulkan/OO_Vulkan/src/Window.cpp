#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>
#include "keycodes.h"
#include "Input.h"
#include "Window.h"

#include "imgui/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


uint64_t Window::SurfaceFormat{};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{


    auto result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    if (result)
        return DefWindowProc(hWnd, uMsg, wParam, lParam);;

	switch (uMsg)
	{
    case WM_SYSCHAR:
    return true;
    break;
    case WM_PAINT:
    ValidateRect(hWnd, NULL);
    break;
	case WM_CLOSE:
	//PostQuitMessage(0);
	case WM_QUIT:
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (window)
		{
			window->windowShouldClose = true;
		}
        return true;
	}
	break;
	case WM_SIZE:
	{
		uint32_t width = LOWORD(lParam);
		uint32_t height = HIWORD(lParam);		
        Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		switch (wParam)
		{
		case SIZE_MAXHIDE: // Message is sent to all pop-up windows when some other window is maximized.
		break;
		case SIZE_MAXIMIZED: //The window has been maximized.
		{
			//    std::cout << "Window maximized" << std::endl;

		}
		break;
		case  SIZE_MAXSHOW: //Message is sent to all pop-up windows when some other window has been restored to its former size.
		//std::cout << "Window max show" << std::endl;
		break;
		case SIZE_MINIMIZED: //The window has been minimized.
		width = 0;
		height = 0;
		//std::cout << "Window minimized" << std::endl;
		break;
		case SIZE_RESTORED: //The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies.
		{
			//    std::cout << "Window restored" << std::endl;
		}
		break;
		}

		if (window)
		{
			window->m_width = width;
			window->m_height = height;
		    std::cout << "Window size changed to ["<< width << "," << height << "]" << std::endl;
		}
	}
	break;
	case WM_KEYDOWN:
	{
		Input::keysTriggered[wParam] = true;
		Input::keysHeld[wParam] = true;

		//std::cout << "Key Pressed" << wParam<<std::endl;
        return true;
	}
	break;
    case WM_KEYUP:
    {
        Input::keysRelease[wParam] = true;
        Input::keysHeld[wParam] = false;

       // std::cout << "Key Release\n";
        return true;
    }
    break;
    case WM_MOUSEMOVE:
    {
        Input::HandleMouseMove(LOWORD(lParam), HIWORD(lParam));
        break;
    }
    case WM_LBUTTONDOWN:
    Input::mouseButtonTriggered[Input::MouseButton::left] = true;
    Input::mouseButtonHeld[Input::MouseButton::left] = true;
    break;
    case WM_RBUTTONDOWN:
    Input::mouseButtonTriggered[Input::MouseButton::right] = true;
    Input::mouseButtonHeld[Input::MouseButton::right] = true;
    break;
    case WM_MBUTTONDOWN:
    Input::mouseButtonTriggered[Input::MouseButton::middle] = true;
    Input::mouseButtonHeld[Input::MouseButton::middle] = true;
    break;
    case WM_LBUTTONUP:
    Input::mouseButtonRelease[Input::MouseButton::left] = true;
    Input::mouseButtonHeld[Input::MouseButton::left] = false;
    break;
    case WM_RBUTTONUP:
    Input::mouseButtonRelease[Input::MouseButton::right] = true;
    Input::mouseButtonHeld[Input::MouseButton::right] = false;
    break;
    case WM_MBUTTONUP:
    Input::mouseButtonRelease[Input::MouseButton::middle] = true;
    Input::mouseButtonHeld[Input::MouseButton::middle] = false;
    break;
    case WM_MOUSEWHEEL:
    {
        short wd = GET_WHEEL_DELTA_WPARAM(wParam);
        Input::wheelDelta = wd;
    }
    break;

	}    // End switch

	// Pass Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


Window::Window(uint32_t width, uint32_t height, WindowType type):
    m_width{width},
    m_height{height},
    m_type{ type },
    rawHandle{NULL},
    windowShouldClose{false}
{
}

Window::~Window()
{
    if (m_type == WindowType::WINDOWS32 && rawHandle)
    {
        DestroyWindow((HWND)rawHandle);
        rawHandle = NULL;
    }
}

void Window::Init()
{
    const auto hInstance = GetModuleHandle(NULL); // setting as null makes windows give us the hdl ptr of this app
                                                  // ------------------------------------------ // you can set it to exe names to get their handle at runtime

                                                  // Check if we already register the class in WinOS.
                                                  // This performs a check to see if we have already registered this class with WinOS.
    {
        // NOTE: the TEXT() macro helps to convert between strings and Wstrings where needed
        WNDCLASSEX C{};
        if (GetClassInfoEx(hInstance, TEXT("WindowRegistrationHash"), &C))
        {
            // xGPU returns nullptr here because there exists a class definition in the WinOS so we can proceed with creation.
            //return nullptr;
        }
    }

    // to startup we need a window class struct with all the info of window creation
    // most of this stuff is generic settings for the window
    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW; // redraw the window on horz and vert resizing
    wndClass.lpfnWndProc = WindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(DKGRAY_BRUSH)); // window background color
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = TEXT("WindowRegistrationHash");
    wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);


    // Register class performs the registration of the window class in the system.
    // This sort of initializes a "template" for WinOS to create windows from.
    if (!RegisterClassEx(&wndClass))
    {
        // print error
        //return VGPU_ERROR(xgpu::device::error::FAILURE, "Could not register window class!" );
    }


    //system metrics macro allows you to get the metrix of the main monitor.
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // this will probably application wide settings for full screen
    static bool is_fullScreen = false;

    // this is honestly some extra stuff to deal with winOS specific windows management
    if (is_fullScreen)
    {
        DEVMODE dmScreenSettings{};
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = screenWidth;
        dmScreenSettings.dmPelsHeight = screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // perform a check to see if the window width and height are not the screen height
        if ((m_width != screenWidth) && (m_height != screenHeight))
        {
            // what change display settings does it inform WinOS that it wishes to set the system resolution to this.
            // CSD_FULLSCREEN tells WinOS this change is temporary while application is focused. 
            if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            {

                //return VGPU_ERROR( xgpu::device::error::FAILURE, "Fullscreen Mode not supported!");
            }
        }
    }

    // windows style customization
    const DWORD dwExStyle = is_fullScreen ? WS_EX_APPWINDOW : WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    const DWORD dwStyle = is_fullScreen ? WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN : WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    RECT windowRect;
    // setting desired size for window
    if (is_fullScreen)
    {
        windowRect.left = static_cast<LONG>(0);
        windowRect.right = static_cast<LONG>(screenWidth);
        windowRect.top = static_cast<LONG>(0);
        windowRect.bottom = static_cast<LONG>(screenHeight);
    }
    else
    {
        windowRect.left = static_cast<LONG>(screenWidth) / 2 - m_width / 2;
        windowRect.right = static_cast<LONG>(m_width);
        windowRect.top = static_cast<LONG>(screenHeight) / 2 - m_height / 2;
        windowRect.bottom = static_cast<LONG>(m_height);
    }

    // this function calculates the window's size that the system will use. 
    // Its literally a conversion function that returns the recalculated coords into the rect struct.
    AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);
    // we will use this calcualted rect struct to create the window next

    //now we can finally get the window
    rawHandle = CreateWindowEx(0
        , TEXT("WindowRegistrationHash") // the class registration we done previously. MUST MATCH
        , TEXT("Window Name")  // window name goes here
        , dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
        , windowRect.left
        , windowRect.top
        , windowRect.right
        , windowRect.bottom
        , NULL
        , NULL
        , hInstance
        , NULL
    );

    if (rawHandle == NULL)
    {
        throw std::runtime_error("Failed to create a window!");
    }

    //window now exists we need to show the window..
    ShowWindow((HWND)rawHandle, SW_SHOW); // theres a bunch of cool commands @ https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
    SetForegroundWindow((HWND)rawHandle);
    SetFocus((HWND)rawHandle);
    // window is now created!
    UpdateWindow((HWND)rawHandle);

    /*  // This function is interesting. You can kind of "Embed" information into the window as a ptr.
    // So you can ptr to the encapsulating window class in C++ and retrieve it later using GetWindowLongPtr()
    */
    if (rawHandle)
    {
        SetWindowLongPtr((HWND)rawHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

}

HWND Window::GetRawHandle()const
{
    return (HWND)rawHandle;
}

bool Window::PollEvents()
{
    MSG msg;


    // 1. Check if theres a message in the WinOS message queue and remove it using PM_REMOVE
    if(PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) // process every single message in the queue
    {
            // Parses and translates the message for WndProc function
            TranslateMessage(&msg);
            // now we dispatch the compatible message to our WndProc function.
            DispatchMessage(&msg);
            return true;
    }
    return false;
}
