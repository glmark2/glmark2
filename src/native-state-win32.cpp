/*
 * Copyright © 2019 Jamie Madill
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Jamie Madill
 */
#include "native-state-win32.h"
#include "log.h"

static const char *win_name("glmark2 " GLMARK_VERSION);
static const char *child_win_name("glmark2 " GLMARK_VERSION " child");

/******************
 * Public methods *
 ******************/

NativeStateWin32::NativeStateWin32() : parent_window_(0), child_window_(0), native_display_(0), should_quit_(false) {}

NativeStateWin32::~NativeStateWin32() { cleanup(); }

bool
NativeStateWin32::init_display()
{
    cleanup();

    static const LONG initial_width = 128;
    static const LONG initial_height = 128;

    // Work around compile error from not defining "UNICODE" while Chromium does
    const LPSTR idcArrow = MAKEINTRESOURCEA(32512);

    WNDCLASSEXA parentWindowClass = {};
    parentWindowClass.cbSize = sizeof(WNDCLASSEXA);
    parentWindowClass.style = 0;
    parentWindowClass.lpfnWndProc = wnd_proc;
    parentWindowClass.cbClsExtra = 0;
    parentWindowClass.cbWndExtra = 0;
    parentWindowClass.hInstance = GetModuleHandle(nullptr);
    parentWindowClass.hIcon = nullptr;
    parentWindowClass.hCursor = LoadCursorA(nullptr, idcArrow);
    parentWindowClass.hbrBackground = 0;
    parentWindowClass.lpszMenuName = nullptr;
    parentWindowClass.lpszClassName = win_name;
    if (!RegisterClassExA(&parentWindowClass))
    {
        return false;
    }

    WNDCLASSEXA childWindowClass = {};
    childWindowClass.cbSize = sizeof(WNDCLASSEXA);
    childWindowClass.style = CS_OWNDC;
    childWindowClass.lpfnWndProc = wnd_proc;
    childWindowClass.cbClsExtra = 0;
    childWindowClass.cbWndExtra = 0;
    childWindowClass.hInstance = GetModuleHandle(nullptr);
    childWindowClass.hIcon = nullptr;
    childWindowClass.hCursor = LoadCursorA(nullptr, idcArrow);
    childWindowClass.hbrBackground = 0;
    childWindowClass.lpszMenuName = nullptr;
    childWindowClass.lpszClassName = child_win_name;
    if (!RegisterClassExA(&childWindowClass))
    {
        return false;
    }

    DWORD parentStyle = WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
    DWORD parentExtendedStyle = WS_EX_APPWINDOW;

    RECT sizeRect = { 0, 0, initial_width, initial_height };
    AdjustWindowRectEx(&sizeRect, parentStyle, FALSE, parentExtendedStyle);

    parent_window_ = CreateWindowExA(parentExtendedStyle, win_name, win_name,
        parentStyle, CW_USEDEFAULT, CW_USEDEFAULT,
        sizeRect.right - sizeRect.left, sizeRect.bottom - sizeRect.top,
        nullptr, nullptr, GetModuleHandle(nullptr), this);
    intermediate_window_ = CreateWindowExA(0, child_win_name, win_name, WS_CHILD,
        0, 0, static_cast<int>(initial_width), static_cast<int>(initial_height),
        parent_window_, nullptr, GetModuleHandle(nullptr), this);
    child_window_ = CreateWindowExA(0, child_win_name, win_name, WS_CHILD,
        static_cast<int>(initial_width/4), static_cast<int>(initial_height/4),
        static_cast<int>(initial_width/2), static_cast<int>(initial_height/2),
        intermediate_window_, nullptr, GetModuleHandle(nullptr), this);

    native_display_ = GetDC(child_window_);
    if (!native_display_)
    {
        cleanup();
    }

    return native_display_;
}

void*
NativeStateWin32::display()
{
    return native_display_;
}

bool
NativeStateWin32::create_window(WindowProperties const& properties)
{
    properties_ = properties;

    RECT windowRect;
    if (!GetWindowRect(parent_window_, &windowRect))
    {
        return false;
    }

    RECT clientRect;
    if (!GetClientRect(parent_window_, &clientRect))
    {
        return false;
    }

    LONG diffX = (windowRect.right - windowRect.left) - clientRect.right;
    LONG diffY = (windowRect.bottom - windowRect.top) - clientRect.bottom;
    if (!MoveWindow(parent_window_, windowRect.left, windowRect.top, properties.width + diffX, properties.height + diffY,
        TRUE))
    {
        return false;
    }

    if (!MoveWindow(intermediate_window_, 0, 0, properties.width, properties.height, FALSE))
    {
        return false;
    }

    if (!MoveWindow(child_window_, properties.width/4, properties.height/4, properties.width/2, properties.height/2, FALSE))
    {
        return false;
    }

    return true;
}

void*
NativeStateWin32::window(WindowProperties& properties)
{
    properties = properties_;
    return child_window_;
}

void
NativeStateWin32::visible(bool visible)
{
    int flag = (visible ? SW_SHOW : SW_HIDE);

    if (parent_window_)
    {
        ShowWindow(parent_window_, flag);
    }

    if (intermediate_window_)
    {
        ShowWindow(intermediate_window_, flag);
    }
    if (child_window_)
    {
        ShowWindow(child_window_, flag);
    }
}

bool
NativeStateWin32::should_quit()
{
    pump_messages();
    return should_quit_;
}

void
NativeStateWin32::flip()
{
    pump_messages();
}

/*******************
 * Private methods *
 *******************/

void
NativeStateWin32::cleanup()
{
    if (native_display_)
    {
        ReleaseDC(child_window_, native_display_);
        native_display_ = 0;
    }

    if (child_window_)
    {
        DestroyWindow(child_window_);
        child_window_ = 0;
    }

    if (parent_window_)
    {
        DestroyWindow(parent_window_);
        parent_window_ = 0;
    }

    UnregisterClassA(win_name, nullptr);
    UnregisterClassA(child_win_name, nullptr);
}

void
NativeStateWin32::pump_messages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK
NativeStateWin32::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_NCCREATE:
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            return DefWindowProcA(hWnd, message, wParam, lParam);
        }
    }

    NativeStateWin32 *window = reinterpret_cast<NativeStateWin32 *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (window)
    {
        switch (message)
        {
            case WM_CLOSE:
            case WM_DESTROY:
                window->should_quit_ = true;
                break;
            case WM_KEYDOWN:
                {
                    HWND target;
                    switch (wParam)
                    {
                        case '1': target = window->parent_window_; break;
                        case '2': target = window->intermediate_window_; break;
                        case '3': target = window->child_window_; break;
                        default: target = NULL;
                    }
                    LONG style = GetWindowLong(target, GWL_STYLE);
                    if (style & WS_VISIBLE)
                    {
                        Log::info("[glmark2] Hiding %p\n", target);
                        ShowWindow(target, SW_HIDE);
                        Log::info("[glmark2] Hiding %p done\n", target);
                    }
                    else
                    {
                        Log::info("[glmark2] Showing %p\n", target);
                        ShowWindow(target, SW_SHOW);
                        Log::info("[glmark2] Showing %p done\n", target);
                    }
                }
                break;
            case WM_LBUTTONDOWN:
                Log::info("[glmark2] Moving %p to 0,0\n", window->child_window_);
                SetWindowPos(window->child_window_, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                Log::info("[glmark2] Moving %p to 0,0 done\n", window->child_window_);
                break;
            case WM_RBUTTONDOWN:
                Log::info("[glmark2] Moving %p to %d,%d\n", window->child_window_,
                          window->properties_.width / 2, window->properties_.height/2);
                SetWindowPos(window->child_window_, 0, window->properties_.width / 2, window->properties_.height/2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                Log::info("[glmark2] Moving %p to %d,%d done\n", window->child_window_,
                          window->properties_.width / 2, window->properties_.height/2);
                break;
            case WM_MBUTTONDOWN:
                {
                    LONG style = GetWindowLong(window->intermediate_window_, GWL_STYLE);
                    if (style & WS_CHILD)
                    {
                        Log::info("[glmark2] Making %p toplevel (SetParent)\n", window->intermediate_window_);
                        SetParent(window->intermediate_window_, NULL);
                        style &= ~WS_CHILD;
                        style |= WS_POPUPWINDOW | WS_CAPTION;
                        Log::info("[glmark2] Making %p toplevel 2 SetWindowLong\n", window->intermediate_window_);
                        SetWindowLong(window->intermediate_window_, GWL_STYLE, style);
                        Log::info("[glmark2] Making %p toplevel 3 SetWindowPos\n", window->intermediate_window_);
                        SetWindowPos(window->intermediate_window_, 0, 500, 500, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                        Log::info("[glmark2] Making %p toplevel done\n", window->intermediate_window_);
                    }
                    else
                    {
                        Log::info("[glmark2] Making %p child (SetWindowLong)\n", window->intermediate_window_);
                        style |= WS_CHILD;
                        style &= ~WS_POPUPWINDOW;
                        style &= ~WS_CAPTION;
                        SetWindowLong(window->intermediate_window_, GWL_STYLE, style);
                        Log::info("[glmark2] Making %p child 2 SetParent\n", window->intermediate_window_);
                        SetParent(window->intermediate_window_, window->parent_window_);
                        Log::info("[glmark2] Making %p child 3 SetWindowPos\n", window->intermediate_window_);
                        SetWindowPos(window->intermediate_window_, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                        Log::info("[glmark2] Making %p child done\n", window->intermediate_window_);
                    }
                }
                break;
            default:
                break;
        }
    }

    return DefWindowProcA(hWnd, message, wParam, lParam);
}
