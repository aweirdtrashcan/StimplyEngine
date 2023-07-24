#include "stdafx.h"
#include "Window.h"
#include "EngineTypes.inl"
#include "EngineGlobals.h"
#include <stdarg.h>
#include <string.h>
#include "Application.h"
#include "Event.h"
#include "Renderer.h"
#include <string>
#include <ostream>
#include <sstream>
#include "utils.h"
#include <imgui.h>
#include <backends/imgui_impl_win32.h>

#pragma warning(disable : 4302)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window(HINSTANCE hInstance, Vector2D initialPos, Vector2D initialSize, const wchar_t* className)
	:
	_initialSize(initialSize)
{
	int result;

	WNDCLASSEXW wnd = {};
	wnd.cbSize = sizeof(WNDCLASSEXW);
	wnd.style = 0;
	wnd.lpfnWndProc = &Window::WindowHandleSetup;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = hInstance;
	wnd.hIcon = LoadIconW(0, 0);
	wnd.hCursor = LoadCursorW(0, MAKEINTRESOURCEW(IDC_ARROW));
	wnd.hbrBackground = 0;
	wnd.lpszMenuName = 0;
	wnd.lpszClassName = className;
	wnd.hIconSm = LoadIconW(0, 0);

	result = RegisterClassExW(&wnd);
	
	assert(result && "Failed to register application's window");

	DWORD exStyle = WS_EX_TRANSPARENT | WS_EX_APPWINDOW;
	DWORD style = WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME | WS_OVERLAPPEDWINDOW;

	RECT rect{};
	rect.left = 0;
	rect.top -= 0;
	rect.right += (LONG)initialSize.x;
	rect.bottom += (LONG)initialSize.y;

	BOOL res = AdjustWindowRect(
		&rect,
		WS_OVERLAPPEDWINDOW,
		FALSE
	);

	if (!res)
	{
		throw std::exception();
	}

	LONG windowWidth = rect.right - rect.left;
	LONG windowHeight = rect.bottom - rect.top;

	_hWnd = CreateWindowExW(
		exStyle,
		className,
		L"Stimply Engine",
		style,
		(int)initialPos.x,
		(int)initialPos.y,
		windowWidth,
		windowHeight,
		0,
		0,
		hInstance,
		this
	);

	assert(_hWnd && "Failed to create application's window");

	_windowRect = rect;

	ShowWindow(_hWnd, SW_SHOW);

#ifdef _DEBUG
	s_Console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#else
	FreeConsole();
#endif

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	_className = className;
	_hInstance = hInstance;
}

Window::~Window()
{
	Global::gIsRunning = false;
	UnregisterClassW(_className, _hInstance);
	DestroyWindow(_hWnd);
}

void Window::Throw(HRESULT reason)
{
	std::ostringstream errorMsg;

	errorMsg << TranslateDX11Error(reason) <<
		": " << std::hex << reason << std::endl;

	std::string stdstr = errorMsg.str();
	const char* str = stdstr.c_str();

	Log(ConsoleColor::Red, str);

	MessageBoxA(0, str, "Window Error Occurred", MB_OK);
	
	throw std::exception();
}

void Window::Log(ConsoleColor color, const char* message, ...)
{
#ifdef _DEBUG
	if (s_Console_handle == nullptr) return;
	WORD backgroundAttrib = 0;
	WORD standardColor = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED;

	switch (color)
	{
	case ConsoleColor::Red:
	{
		backgroundAttrib |= FOREGROUND_RED | FOREGROUND_INTENSITY;
		break;
	}
	case ConsoleColor::Blue:
	{
		backgroundAttrib |= FOREGROUND_BLUE;
		break;
	}
	case ConsoleColor::Green:
	{
		backgroundAttrib |= FOREGROUND_GREEN;
		break;
	}
	case ConsoleColor::Standard:
	{
		backgroundAttrib |= standardColor;
		break;
	}
	}

	char tempBuffer[1024];
	memset(tempBuffer, 0, 1024);
	
	va_list vararg_ptr;
	va_start(vararg_ptr, message);
	vsnprintf(tempBuffer, 1024, message, vararg_ptr);
	va_end(vararg_ptr);

	SetConsoleTextAttribute(s_Console_handle, backgroundAttrib);
	printf("%s\n", tempBuffer);
	SetConsoleTextAttribute(s_Console_handle, standardColor);
#endif
}

LRESULT Window::WindowHandleSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		CREATESTRUCTW* createStruct = (CREATESTRUCTW*)lParam;
		Window* window = (Window*)createStruct->lpCreateParams;
		
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)window);

		createStruct->lpCreateParams = 0;

		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	Window* window = (Window*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	return window->HandleMessages(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return TRUE;
	}

	switch (msg)
	{
	case WM_CLOSE:
	{
		// if gIsRunning is false, whole application closes.
		Global::gIsRunning = false;
		return TRUE;
	} break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
		{
			_keyboard.SetKeyPressed((unsigned char)wParam);
		}
		else
		{
			_keyboard.SetKeyReleased((unsigned char)wParam);
		}
		return 0;
	} break;
	case WM_LBUTTONDOWN:
		return 0;
	case WM_RBUTTONDOWN:
		return 0;
	case WM_MBUTTONDOWN:
		return 0;
	case WM_LBUTTONUP:
		return 0;
	case WM_RBUTTONUP:
		return 0;
	case WM_MBUTTONUP:
		return 0;
	case WM_MOUSEMOVE:
		/*POINTS pt = MAKEPOINTS(lParam);
		Log(ConsoleColor::Red, "%d %d", (int)pt.x, (int)pt.y);*/
		return 0;
	case WM_MOUSEWHEEL:
		return 0;
	case WM_ERASEBKGND:
	{
		return TRUE;
	}
	case WM_SIZE:
	{
		RECT clientRect{};
		GetClientRect(hWnd, &clientRect);

		uint32_t width = (uint32_t)clientRect.right - clientRect.left;
		uint32_t height = (uint32_t)clientRect.bottom - clientRect.top;

		uint32_t size[] = { width, height };

		Event::Trigger(EventType::WINDOW_RESIZE, (void*)size);
	}
	default:
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}
