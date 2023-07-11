#include "misc.h"

#include <windows.h>

const int WIDTH = 1920;
const int HEIGHT = 1080;

typedef struct Tag_Window {
	int width;
	int height;
	HINSTANCE instance;
	HWND handle;
	b8 should_close;
} Window;

static Window window = {0};

void resize_dib_section() {
	
}



LRESULT CALLBACK main_window_callback(HWND w_handle, UINT message, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			window.width = LOWORD(lparam);
			window.height = HIWORD(lparam);
			resize_dib_section();
		} break;

		case WM_CLOSE: {
			window.should_close = TRUE;
		} break;

		case WM_ACTIVATEAPP: {
			// stop player from walking when tabbing out while pressing down any of the movement keys
			// the key doesn't get set to false since Windows doesn't send an event to the window when
			// it's out of focus
			if (wparam == FALSE) { // window loses focus
				//reset_keyboard_state();
			}
		} break;

		case WM_DESTROY: {
			window.should_close = TRUE;
		} break;

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window.handle, &paint);
			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			static DWORD operation = WHITENESS;
			PatBlt(device_context, x, y, width, height, operation);
			if (operation == WHITENESS) {
				operation = BLACKNESS;
			} 
			else {
				operation = WHITENESS;
			}
			EndPaint(window.handle, &paint);
		} break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			// @todo: shouldn't happen since we do handle this before this gets called
		} break;

		default: {
			result = DefWindowProc(w_handle, message, wparam, lparam);
		} break;
	}

	return result;
}

b8 platform_create_window(const char *title, int width, int height) {
	WNDCLASSA w_class = {0};
	w_class.style = CS_HREDRAW | CS_VREDRAW;
	w_class.lpfnWndProc = main_window_callback;
	w_class.hInstance = window.instance;
	w_class.hCursor = LoadCursor(0, IDC_ARROW);
	//w_class.hIcon = ;
	w_class.lpszClassName = "3drendererwindowclass";

	if (!RegisterClassA(&w_class)) {
		return FALSE;
	}

	HWND w_handle = CreateWindowExA(
		0,
		w_class.lpszClassName,
		title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100,
		width, height,
		0,
		0,
		w_class.hInstance,
		0);

	if (!w_handle) {
		return FALSE;
	}

	window.handle = w_handle;

	return TRUE;
}

void platform_process_events() {
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		switch (message.message) {
			case WM_QUIT: {
				window.should_close = TRUE;
			} break;

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP: {
				WORD vk_code = LOWORD(message.wParam);
				WORD key_flags = HIWORD(message.lParam);

				b8 alt_down = (key_flags & KF_ALTDOWN) == KF_ALTDOWN;

				switch(vk_code) {
					case VK_F4: {
						if (alt_down) window.should_close = TRUE;
					} break;
						
					default: {
						// do nothing
					} break;
				}
			} break;

			default: {
				TranslateMessage(&message);
				DispatchMessage(&message);
			} break;
		}
	}
}

void update() {

}

void render() {
	
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {
	LARGE_INTEGER perf_count_frequency_result;
	QueryPerformanceFrequency(&perf_count_frequency_result);
	i64 perf_count_frequency = perf_count_frequency_result.QuadPart;

	window.width = WIDTH;
	window.height = HEIGHT;
	window.instance = instance;
	window.should_close = TRUE;

	int result = platform_create_window("3drenderer", window.width, window.height);
	if (result != TRUE) {
		return result;
	}

	window.should_close = FALSE;

	LARGE_INTEGER last_counter;
	QueryPerformanceCounter(&last_counter);
	u64 last_cycle_count = __rdtsc();

	double frame_time = 0.0;
	double fps = 0.0;
	double mcpf = 0.0;

	while (!window.should_close) {
		float delta_time = (float)frame_time / 1000.0f;

		platform_process_events();

		update();
		render();

		u64 end_cycle_count = __rdtsc();
		LARGE_INTEGER end_counter;
		QueryPerformanceCounter(&end_counter);
		
		u64 cycles_elapsed  = end_cycle_count - last_cycle_count;
		i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart;
		frame_time = (1000.0 * (double)counter_elapsed) / (double)perf_count_frequency;
		fps = (double)perf_count_frequency / (double)counter_elapsed;
		mcpf = (double)cycles_elapsed / (1000.0 * 1000.0); // mcpf == million cycles per frame

		last_counter = end_counter;
		last_cycle_count = end_cycle_count;
	}

	// @todo: free stuff
	
	return 0;
}