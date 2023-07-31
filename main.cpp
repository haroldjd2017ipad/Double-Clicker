#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex mutex;

bool Enabled = false;
bool acceptInput = true;
HWND window;

HHOOK keyboardHook;
HHOOK mouseHook;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

void EnableConsoleOutput();


void click()
{
	std::this_thread::sleep_for(std::chrono::duration<double>(0.01));
	INPUT input = { 0 };
	POINT pos;

	GetCursorPos(&pos);

	input.mi.dx = pos.x;
	input.mi.dy = pos.y;
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;

	acceptInput = false;
	SendInput(1, &input, sizeof(INPUT));
	acceptInput = true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//EnableConsoleOutput();
	SetProcessDPIAware();

	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
	mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
	WNDCLASSEX wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = hInstance;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszClassName = _T("WINDOW");
	wcex.lpfnWndProc = WndProc;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("failed to create window class"), _T("Double Clicker"), MB_ICONERROR);
		return 1;
	}

	window = CreateWindowEx(
		WS_EX_TOPMOST,
		_T("WINDOW"),
		_T("Double Clicker"),
		WS_OVERLAPPED | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		0, 95,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(window, nCmdShow);
	UpdateWindow(window);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(keyboardHook);
	UnhookWindowsHookEx(mouseHook);
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 16, 0, _T("Shift + R"), 9);
		TextOut(hdc, 16, 17, _T("status:"), 7);
		SetTextColor(hdc, Enabled ? RGB(0, 200, 0) : RGB(255, 0, 0));
		TextOut(hdc, 75, 17, Enabled ? _T("ENABLED") : _T("DISABLED"), Enabled ? 7 : 8);
		EndPaint(hWnd, &ps);

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	static bool shift = false;

	if (nCode != HC_ACTION)
		return 0;

	KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
	DWORD vkCode = kbStruct->vkCode;

	if (vkCode == 160 || vkCode == 161)
	{
		shift = ((int)wParam == 256);
	}

	if (vkCode == 82 && (int)wParam == 256 && shift)
	{
		Enabled = !Enabled;
		InvalidateRect(window, NULL, TRUE);
	}

	return 0;
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION || !Enabled || (int)wParam != 514 || !acceptInput)
		return 0;

	std::thread t(click);
	t.detach();

	return 0;
}

void EnableConsoleOutput()
{
	AllocConsole();
	HANDLE consoleHandle = CreateFile(_T("CONOUT$"), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (consoleHandle != INVALID_HANDLE_VALUE)
	{
		SetStdHandle(STD_OUTPUT_HANDLE, consoleHandle);
		FILE* file = nullptr;
		freopen_s(&file, "CONOUT$", "w", stdout);
		std::ios::sync_with_stdio();
	}
}