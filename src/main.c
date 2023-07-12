#include "misc.h"

#include <windows.h>

const int WIDTH = 1440;
const int HEIGHT = 810;

typedef struct Tag_Window {
	int width;
	int height;
	HINSTANCE instance;
	HWND handle;
} Window;

typedef struct Tag_Offscreen_Buffer {
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	int bytes_per_pixel;
} Offscreen_Buffer;

static b8 should_close = TRUE;
static Window window; // @todo: un-global this
static Offscreen_Buffer global_backbuffer;

void render_weird_gradient(Offscreen_Buffer buffer, int xoffset, int yoffset) {
	u8 *row = (u8 *)buffer.memory;
	for (int y = 0; y < buffer.height; ++y) {
		u32 *pixel = (u32 *)row;
		for (int x = 0; x < buffer.width; ++x) {
			u8 blue  = x + xoffset;
			u8 green = y + yoffset;

			*pixel++ = (green << 8) | blue;
		}
		row += buffer.pitch;
	}
}

void resize_dib_section(Offscreen_Buffer *buffer, int width, int height) {
	if (buffer->memory) {
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}

	buffer->width  = width;
	buffer->height = height;
	buffer->bytes_per_pixel = 4;

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height; // '-' becaues I want top down dib (origin at top left corner)
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	//bitmap_info.bmiHeader.biSizeImage = 0;
	//bitmap_info.bmiHeader.biXPelsPerMeter = 0;
	//bitmap_info.bmiHeader.biYPelsPerMeter = 0;
	//bitmap_info.bmiHeader.biClrUsed = 0;
	//bitmap_info.bmiHeader.biClrImportant = 0;

	int bitmap_memory_size = buffer->bytes_per_pixel * buffer->width * buffer->height;
	buffer->memory = VirtualAlloc(NULL, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);

	// @todo: probably clear to black

	buffer->pitch = buffer->width * buffer->bytes_per_pixel;
}

void copy_buffer_to_display(HDC device_context, Offscreen_Buffer buffer, Window window, int x, int y, int width, int height) {
	StretchDIBits(
		device_context,
		0, 0, buffer.width, buffer.height, // destination
		0, 0, width, height, // source
		buffer.memory,
		&buffer.info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK main_window_callback(HWND w_handle, UINT message, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			int width = LOWORD(lparam);
			int height = HIWORD(lparam);
			resize_dib_section(&global_backbuffer, width, height); // dib == device independent bitmap
		} break;

		case WM_CLOSE: {
			should_close = TRUE;
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
			should_close = TRUE;
		} break;

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(w_handle, &paint);
			//int x = paint.rcPaint.left;
			//int y = paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			copy_buffer_to_display(device_context, global_backbuffer, window, 0, 0, width, height);
			EndPaint(w_handle, &paint);
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
				should_close = TRUE;
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
						if (alt_down) should_close = TRUE;
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

	int result = platform_create_window("3drenderer", window.width, window.height);
	if (result != TRUE) {
		return result;
	}

	should_close = FALSE;

	LARGE_INTEGER last_counter;
	QueryPerformanceCounter(&last_counter);
	u64 last_cycle_count = __rdtsc();

	double frame_time = 0.0;
	double fps = 0.0;
	double mcpf = 0.0;

	while (!should_close) {
		float delta_time = (float)frame_time / 1000.0f;

		platform_process_events();

		static int xoffset = 0;
		static int yoffset = 0;
		render_weird_gradient(global_backbuffer, xoffset, yoffset);

		HDC device_context = GetDC(window.handle);
		copy_buffer_to_display(device_context, global_backbuffer, window, 0, 0, global_backbuffer.width, global_backbuffer.height);
		ReleaseDC(window.handle, device_context);

		++xoffset;
		yoffset += 2;

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