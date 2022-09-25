#include "WindowContainer.h"

bool RenderWindow::Initialize(WindowContainer * pWindowContainer, HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height) {
	this->hInstance = hInstance;
	this->width = width;
	this->height = height;
	this->window_title = window_title;
	this->window_title_wide = StringHelper::StringToWide(this->window_title);
	this->window_class = window_class;
	this->window_class_wide = StringHelper::StringToWide(this->window_class); // wide string representatin of class window

	this->RegisterWindowClass();

	// Get center of screen
	int centerScreenX = GetSystemMetrics(SM_CXSCREEN) / 2 - this->width / 2;
	int centerScreenY = GetSystemMetrics(SM_CYSCREEN) / 2 - this->height / 2;

	RECT wr; // Window rectangle
	wr.left = centerScreenX;
	wr.top = centerScreenY;
	wr.right = wr.left + this->width;
	wr.bottom = wr.top + this->height;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	this->handle = CreateWindowEx(0, //Extended windows style - we are using the default)
		this->window_class_wide.c_str(), // Window class name
		this->window_title_wide.c_str(), // Window Title
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, // Windows style
		wr.left, // Window X position
		wr.top, // Window Y Position
		wr.right - wr.left, // Window Width
		wr.bottom - wr.top, // Window Height
		NULL, // Handle to parent of this window. Since this is the first window, it has no parent window.
		NULL, // Handle to menu or child window identifier. Can be set to NULL and use menu in WindowClassEx if a 
		this->hInstance, // Handle to the instance of module to be used with this window
		pWindowContainer); // Param to create window

	if (this->handle == NULL) {
		ErrorLogger::Log(GetLastError(), "CreateWindowEX Failed for window: " + this->window_title);
		return false;
	}

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(this->handle, SW_SHOW);
	SetForegroundWindow(this->handle);
	SetFocus(this->handle);

	return true;
}

bool RenderWindow::ProcessMessages() {
	// Handle the windows messages
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG)); // Initialize the message structure.

	while (PeekMessage(&msg, // Where to store message (if one exists See))
		this->handle, // Handle to the window we are checking messages for
		0, // Minimum filter msg value - We are not filtering the specific messages, but the min/max could be used
		0, // Maximum Filter MSg Value
		PM_REMOVE)) // Remove message after capuring it via PeekMessage 
	{
		TranslateMessage(&msg); // Translate the messages from virtual key messages into character messages so we can
		DispatchMessage(&msg); // Dispatch message to our Window Proc for this window.
	}

	// Check if the window was closed
	if (msg.message == WM_NULL) {
		if (!IsWindow(this->handle)) {
			this->handle = NULL; // Message processing loop takes care of destroying the window
			UnregisterClass(this->window_class_wide.c_str(), this->hInstance);
			return false;
		}
	}

	return true;
}

HWND RenderWindow::GetHWND() const
{
	return this->handle;
}

RenderWindow::~RenderWindow() {
	if (this->handle != NULL) {
		UnregisterClass(this->window_class_wide.c_str(), this->hInstance);
		DestroyWindow(handle);
	}
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// All other messages
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	default:
	{
		// retreive ptr to window class
		WindowContainer* const pWindow = reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		// Forward message to window class handler
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowContainer* pWindow = reinterpret_cast<WindowContainer*>(pCreate->lpCreateParams);
		if (pWindow == nullptr) // Sanity check
		{
			ErrorLogger::Log("Citical Error: Pointer to window container is null during WM_NCREATE.");
			exit(-1);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void RenderWindow::RegisterWindowClass() {
	WNDCLASSEX wc; // Out Window Class (This has to be filled before our window can be created)
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Flags [Redraw on width/height change from resize/movement]
	wc.lpfnWndProc = HandleMessageSetup; // Pointer to Window Proc function for handling messages from this window
	wc.cbClsExtra = 0; //# of extra bytes to allocate following the window-class structure. We are not currently using
	wc.cbWndExtra = 0; //# of extra bytes to allocate following the window instance structure. We are not currently using
	wc.hInstance = this->hInstance; // Handle to the instance that contains the Window Procedure
	wc.hIcon = NULL; // Handle to the class icon. Must be a handle to an icon resource. Not currently using
	wc.hIconSm = NULL; // Handle to small icon for this class. We are not currently using
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Default Cursor - If we leave this null, we have to explicity set
	wc.hbrBackground = NULL; // Handle to the class background brush for the window's backgrond color - 
	wc.lpszMenuName = NULL; // Pointer to a null terminated character string for the menu. We are not using a meny yet
	wc.lpszClassName = this->window_class_wide.c_str(); // Pointer to null terminated string of our class name
	wc.cbSize = sizeof(WNDCLASSEX); // Need to fill in the size of our struct for cbSize
	RegisterClassEx(&wc); // Register the class so that is it usable
}