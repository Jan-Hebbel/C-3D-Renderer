#include "misc.h"

#include "input.h"

#include <windows.h>
#include <dsound.h>

#include <math.h>

//
// constants
//
const int WIDTH = 1280;
const int HEIGHT = 720;

//
// structures
//
typedef struct Tag_Window {
	int width;
	int height;
	HINSTANCE instance;
	HWND handle;
	int canvas_width;
	int canvas_height;
} Window;

typedef struct Tag_Offscreen_Buffer {
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	int bytes_per_pixel;
} Offscreen_Buffer;

typedef struct Tag_Sound_Output {
	int samples_per_second;
	int tone_hz;
	i16 tone_volume;
	u32 running_sample_index;
	int wave_period;
	int bytes_per_sample;
	int secondary_buffer_size;
	int latency_sample_count;
} Sound_Output;

//
// globals
//
static b8 global_should_close = M_TRUE;
static Offscreen_Buffer global_backbuffer;
static Window global_window;
static LPDIRECTSOUNDBUFFER global_sound_buffer;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(Direct_Sound_Create);

b8 init_direct_sound(HWND window, int buffer_size, int samples_per_second) {
	// load the library
	HMODULE dsound_library = LoadLibraryA("dsound.dll");
	if (!dsound_library) {
		return M_FALSE;
	}

	Direct_Sound_Create *direct_sound_create = (Direct_Sound_Create *)GetProcAddress(dsound_library, "DirectSoundCreate");

	if (!direct_sound_create) {
		return M_FALSE;
	}

	LPDIRECTSOUND direct_sound = NULL;
	if (FAILED( direct_sound_create(NULL, &direct_sound, NULL) )) {
		return M_FALSE;
	}

	if (!direct_sound) {
		return M_FALSE;
	}

	if (IDirectSound_SetCooperativeLevel(direct_sound, window, DSSCL_PRIORITY) != DS_OK) {
		return M_FALSE;
	}

	LPDIRECTSOUNDBUFFER primary_buffer;
	DSBUFFERDESC primary_buffer_description = {0};
	primary_buffer_description.dwSize = sizeof(primary_buffer_description);
	primary_buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (IDirectSound_CreateSoundBuffer(direct_sound, &primary_buffer_description, &primary_buffer, NULL) != DS_OK) {
		return M_FALSE;
	}

	WAVEFORMATEX wave_format = {0};
	wave_format.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.nChannels = 2;
	wave_format.nSamplesPerSec = samples_per_second;
	wave_format.wBitsPerSample = 16;
	wave_format.nBlockAlign = wave_format.nChannels * wave_format.wBitsPerSample / 8;
	wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
	wave_format.cbSize = 0;
	if (IDirectSoundBuffer_SetFormat(primary_buffer, &wave_format) != DS_OK) {
		return M_FALSE;
	}

	DSBUFFERDESC secondary_buffer_description = {0};
	secondary_buffer_description.dwSize = sizeof(secondary_buffer_description);
	secondary_buffer_description.dwBufferBytes = buffer_size;
	secondary_buffer_description.lpwfxFormat = &wave_format;
	if (IDirectSound_CreateSoundBuffer(direct_sound, &secondary_buffer_description, &global_sound_buffer, NULL) != DS_OK) {
		return M_FALSE;
	}

	return M_TRUE;
}

void fill_sound_buffer(Sound_Output *sound_output, DWORD byte_to_lock, DWORD bytes_to_write) {
	VOID *region1;
	DWORD region1_size;
	VOID *region2;
	DWORD region2_size;
	if (SUCCEEDED(IDirectSoundBuffer_Lock(global_sound_buffer,
		byte_to_lock,
		bytes_to_write,
		&region1, &region1_size,
		&region2, &region2_size,
		0))) {

		i16 *sample_out = (i16 *)region1;
		DWORD region1_sample_count = region1_size / sound_output->bytes_per_sample;
		for (DWORD sample_index = 0; sample_index < region1_sample_count; ++sample_index) {
			f32 t = TAU * (f32)sound_output->running_sample_index / (f32)sound_output->wave_period;
			f32 sin_value = sinf(t);
			i16 sample_value = (i16)(sin_value * sound_output->tone_volume);
			*sample_out++ = sample_value;
			*sample_out++ = sample_value;

			++sound_output->running_sample_index;
		}

		sample_out = (i16 *)region2;
		DWORD region2_sample_count = region2_size / sound_output->bytes_per_sample;
		for (DWORD sample_index = 0; sample_index < region2_sample_count; ++sample_index) {
			f32 t = TAU * (f32)sound_output->running_sample_index / (f32)sound_output->wave_period;
			f32 sin_value = sinf(t);
			i16 sample_value = (i16)(sin_value * sound_output->tone_volume);
			*sample_out++ = sample_value;
			*sample_out++ = sample_value;

			++sound_output->running_sample_index;
		}

		IDirectSoundBuffer_Unlock(global_sound_buffer, region1, region1_size, region2, region2_size);
	}
}

void render_weird_gradient(Offscreen_Buffer *buffer, int blue_offset, int green_offset) {
	u8 *row = (u8 *)buffer->memory;
	for (int y = 0; y < buffer->height; ++y) {
		u32 *pixel = (u32 *)row;
		for (int x = 0; x < buffer->width; ++x) {
			u8 blue  = x + blue_offset;
			u8 green = y + green_offset;

			*pixel++ = (green << 8) | blue;
		}
		row += buffer->pitch;
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
	buffer->memory = VirtualAlloc(NULL, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// @todo: probably clear to black

	buffer->pitch = buffer->width * buffer->bytes_per_pixel;
}

void copy_buffer_to_display(Offscreen_Buffer *buffer, HDC device_context, int canvas_width, int canvas_height) {
	// @todo: aspect ratio correction
	StretchDIBits(
		device_context,
		0, 0, canvas_width, canvas_height, // destination
		0, 0, buffer->width, buffer->height, // source
		buffer->memory,
		&buffer->info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK main_window_callback(HWND w_handle, UINT message, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			global_window.canvas_width = LOWORD(lparam);
			global_window.canvas_height = HIWORD(lparam);
		} break;

		case WM_CLOSE: {
			global_should_close = M_TRUE;
		} break;

		case WM_ACTIVATEAPP: {
			// stop player from walking when tabbing out while pressing down any of the movement keys
			// the key doesn't get set to M_FALSE since Windows doesn't send an event to the window when
			// it's out of focus
			if (wparam == M_FALSE) { // window loses focus
				reset_keyboard_state();
			}
		} break;

		case WM_DESTROY: {
			global_should_close = M_TRUE;
		} break;

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(w_handle, &paint);
			int width = paint.rcPaint.right - paint.rcPaint.left;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			copy_buffer_to_display(&global_backbuffer, device_context, width, height);
			EndPaint(w_handle, &paint);
		} break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			// @todo: shouldn't happen since we do handle this before this gets called
		} break;

		default: {
			result = DefWindowProcA(w_handle, message, wparam, lparam);
		} break;
	}

	return result;
}

void platform_process_events() {
	Event_Reader event_reader = {0};

	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		switch (message.message) {
			case WM_QUIT: {
				global_should_close = M_TRUE;
			} break;

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP: {
				WORD vk_code = LOWORD(message.wParam);
				WORD key_flags = HIWORD(message.lParam);

				b8 released = (key_flags & KF_UP) == KF_UP;
				b8 is_down = !released;
				b8 repeated = ((key_flags & KF_REPEAT) == KF_REPEAT) && !released;
				b8 alt_down = (key_flags & KF_ALTDOWN) == KF_ALTDOWN;

				Key_State key_state = {
					.is_down = is_down,
					.released = released,
					.repeated = repeated,
					.alt_down = alt_down
				};

				// NOTE: holding down e.g. w and then while still holding w, holding down a will result in vkcode == a
				// to move at the same time with a and w, use: while (vkCode == 'W' && !released)
				// NOTE: to get only the first pressing of a button use: && is_down && !repeated

				switch (vk_code) {
					case 'W': {
						process_key_event(W, key_state, &event_reader);
					} break;

					case 'A': {
						process_key_event(A, key_state, &event_reader);
					} break;

					case 'S': {
						process_key_event(S, key_state, &event_reader);
					} break;

					case 'D': {
						process_key_event(D, key_state, &event_reader);
					} break;

					case VK_ESCAPE: {
						process_key_event(ESCAPE, key_state, &event_reader);
					} break;

					case VK_F4: {
						if (alt_down) global_should_close = M_TRUE;
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
	//
	// setting up performance metrics
	//
	LARGE_INTEGER perf_count_frequency_result;
	QueryPerformanceFrequency(&perf_count_frequency_result);
	i64 perf_count_frequency = perf_count_frequency_result.QuadPart;

	//
	// create window
	//
	global_window.width = WIDTH;
	global_window.height = HEIGHT;
	global_window.instance = instance;

	resize_dib_section(&global_backbuffer, global_window.width, global_window.height);

	WNDCLASSA w_class = {0};
	w_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	w_class.lpfnWndProc = main_window_callback;
	w_class.hInstance = instance;
	w_class.hCursor = LoadCursor(0, IDC_ARROW);
	//w_class.hIcon = ;
	w_class.lpszClassName = "3drendererwindowclass";

	if (!RegisterClassA(&w_class)) {
		return FAILURE;
	}

	global_window.handle = CreateWindowExA(
		0,
		w_class.lpszClassName,
		"3drenderer",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100,
		global_window.width, global_window.height,
		0,
		0,
		w_class.hInstance,
		0);

	if (!global_window.handle) {
		return FAILURE;
	}

	//
	// loop preparation
	//
	HDC device_context = GetDC(global_window.handle);

	global_should_close = M_FALSE;

	LARGE_INTEGER last_counter;
	QueryPerformanceCounter(&last_counter);
	u64 last_cycle_count = __rdtsc();

	double frame_time = 0.0;
	double fps = 0.0;
	double mcpf = 0.0;

	int blue_offset = 0;
	int green_offset = 0;

	Sound_Output sound_output = {0};
	sound_output.samples_per_second = 48000;
	sound_output.tone_hz = 256;
	sound_output.tone_volume = 2000;
	sound_output.wave_period = sound_output.samples_per_second / sound_output.tone_hz;
	sound_output.bytes_per_sample = sizeof(i16) * 2;
	sound_output.secondary_buffer_size = sound_output.samples_per_second * sound_output.bytes_per_sample;
	sound_output.latency_sample_count = sound_output.samples_per_second / 15;

	init_direct_sound(global_window.handle, sound_output.secondary_buffer_size, sound_output.samples_per_second);
	fill_sound_buffer(&sound_output, 0, sound_output.latency_sample_count * sound_output.bytes_per_sample);
	IDirectSoundBuffer_Play(global_sound_buffer, 0, 0, DSBPLAY_LOOPING);

	while (!global_should_close) {
		float delta_time = (float)frame_time / 1000.0f;

		platform_process_events();

		if (get_key_state(W).is_down) {
			green_offset -= 1;
		}
		if (get_key_state(A).is_down) {
			blue_offset -= 1;
		}
		if (get_key_state(S).is_down) {
			green_offset += 1;
		}
		if (get_key_state(D).is_down) {
			blue_offset += 1;
		}
		render_weird_gradient(&global_backbuffer, blue_offset, green_offset);

		copy_buffer_to_display(&global_backbuffer, device_context, global_window.canvas_width, global_window.canvas_height);

		// @test
		DWORD play_cursor;
		DWORD write_cursor;
		if (SUCCEEDED(IDirectSoundBuffer_GetCurrentPosition(global_sound_buffer, &play_cursor, &write_cursor) )) {
			DWORD byte_to_lock = (sound_output.running_sample_index * sound_output.bytes_per_sample) % sound_output.secondary_buffer_size;
			DWORD target_cursor = (play_cursor + (sound_output.latency_sample_count * sound_output.bytes_per_sample)) % sound_output.secondary_buffer_size;
			DWORD bytes_to_write;
			if (byte_to_lock > target_cursor) {
				bytes_to_write = sound_output.secondary_buffer_size - byte_to_lock;
				bytes_to_write += target_cursor;
			}
			else {
				bytes_to_write = target_cursor - byte_to_lock;
			}

			//if (!sound_is_playing) {
			//	bytes_to_write = sound_output.secondary_buffer_size;
			//}

			fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write);
		}

		//update();
		//render();

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
	
	return SUCCESS;
}