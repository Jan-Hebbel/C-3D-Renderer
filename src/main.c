#include "misc.h"
#include "input.h"
#include "my_math.h"

#include <windows.h>
#include <dsound.h>

#include <math.h>

//
// constants
//
#define WIDTH    1024
#define HEIGHT   1024
#define PIXELS_X 128
#define PIXELS_Y 128

//
// structures
//
typedef struct Tag_Window {
	int width;
	int height;
	HINSTANCE instance;
	HWND handle;
	int client_width;
	int client_height;
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

typedef struct Tag_Vertex {
	Vec3 position;
	Vec3 normal;
	Vec2 texcoord;
	Vec4 color;
} Vertex;

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

void RenderTriangleToBuffer(Offscreen_Buffer *buffer, Vec2I v1, Vec2I v2, Vec2I v3) {
    int x_min = MAX(MIN(MIN(v1.x, v2.x), v3.x), 0);
    int y_min = MAX(MIN(MIN(v1.y, v2.y), v3.y), 0);
    int x_max = MIN(MAX(MAX(v1.x, v2.x), v3.x), buffer->width-1);
    int y_max = MIN(MAX(MAX(v1.y, v2.y), v3.y), buffer->height-1);
    
    u32 *pixel = (u32 *)buffer->memory;
    for (int y = y_min; y <= y_max; ++y) {
        for (int x = x_min; x <= x_max; ++x) {
            Vec2I p = { .x = x, .y = y };
            
            Vec3I crossv2p = icross(vec3i_make(p.x - v1.x, p.y - v1.y, 0), vec3i_make(v2.x - v1.x, v2.y - v1.y, 0));
            Vec3I crossv3p = icross(vec3i_make(p.x - v2.x, p.y - v2.y, 0), vec3i_make(v3.x - v2.x, v3.y - v2.y, 0));
            Vec3I crossv1p = icross(vec3i_make(p.x - v3.x, p.y - v3.y, 0), vec3i_make(v1.x - v3.x, v1.y - v3.y, 0));
            
            b8 p_right_of_v1 = crossv1p.z <= 0;
            b8 p_right_of_v2 = crossv2p.z <= 0;
            b8 p_right_of_v3 = crossv3p.z <= 0; 
            
			b8 inside_triangle = p_right_of_v1 && p_right_of_v2 && p_right_of_v3;

            if (inside_triangle) {
				pixel[x + y * buffer->width] = 0x49c44d;
            }
        }
    }
}

void ClearFramebuffer(Offscreen_Buffer *buffer, u32 color) {
    if (!buffer->memory) return;
    
    int bitmap_size = buffer->width * buffer->height;
    
    u32 *pixel = (u32 *)buffer->memory;
    for (int i = 0; i < bitmap_size; ++i) {
        pixel[i] = color;
    }
}

void CreateFramebuffer(Offscreen_Buffer *buffer, int width, int height) {
    if (buffer->memory) {
        return;
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

	buffer->pitch = buffer->width * buffer->bytes_per_pixel;

	ClearFramebuffer(buffer, 0);
}

void CopyBufferToDisplay(Offscreen_Buffer *buffer, HDC device_context, int canvas_width, int canvas_height) {
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
			global_window.client_width = LOWORD(lparam);
			global_window.client_height = HIWORD(lparam);
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
			CopyBufferToDisplay(&global_backbuffer, device_context, width, height);
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

b8 PlatformCreateWindow(HINSTANCE instance, int width, int height, const char *title) {
    global_window.client_width  = width;
    global_window.client_height = height;
	global_window.instance = instance;

    RECT rect = {0};
    rect.right  = width;
    rect.left   = 0;
    rect.bottom = height;
    rect.top    = 0;
    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0)) {
        return M_FALSE;
    }
    
    global_window.width = rect.right - rect.left;
	global_window.height = rect.bottom - rect.top;

	WNDCLASSA w_class = {0};
	w_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	w_class.lpfnWndProc = main_window_callback;
	w_class.hInstance = instance;
	w_class.hCursor = LoadCursor(0, IDC_ARROW);
	//w_class.hIcon = ;
	w_class.lpszClassName = "3drendererwindowclass";

	if (!RegisterClassA(&w_class)) {
		return M_FALSE;
	}

	global_window.handle = CreateWindowExA(
		0,
		w_class.lpszClassName,
		title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100,
		global_window.width, global_window.height,
		0,
		0,
		w_class.hInstance,
		0);

	if (!global_window.handle) {
		return M_FALSE;
	}
	
	return M_TRUE;
}

void platform_process_events(void) {
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

				// @note: holding down e.g. w and then while still holding w, holding down a will result in vkcode == a
				// to move at the same time with a and w, use: while (vkCode == 'W' && !released)
				// @note: to get only the first pressing of a button use: && is_down && !repeated

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

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int cmd_show) {
	//
	// setting up performance metrics
	//
	LARGE_INTEGER perf_count_frequency_result;
	QueryPerformanceFrequency(&perf_count_frequency_result);
	i64 perf_count_frequency = perf_count_frequency_result.QuadPart;

    if (!PlatformCreateWindow(instance, WIDTH, HEIGHT, "3D Software Renderer")) {
        return FAILURE;
    }
    
   	CreateFramebuffer(&global_backbuffer, PIXELS_X, PIXELS_Y);

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

    Vec2I vertices[] = {
        { .x =  64, .y = 20 },
        { .x = 110, .y = 64 },
        { .x =  30, .y = 90 }
    };

	while (!global_should_close) {
        float delta_time = (float)frame_time / 1000.0f;

		platform_process_events();

        ClearFramebuffer(&global_backbuffer, 0x222244);
        
        if (get_key_state(W).is_down) {
			vertices[2].y -= 1;
		}
		if (get_key_state(A).is_down) {
			vertices[2].x -= 1;
		}
		if (get_key_state(S).is_down) {
			vertices[2].y += 1;
		}
		if (get_key_state(D).is_down) {
			vertices[2].x += 1;
		}
        RenderTriangleToBuffer(&global_backbuffer, vertices[0], vertices[1], vertices[2]);
        CopyBufferToDisplay(&global_backbuffer, device_context, global_window.client_width, global_window.client_height);

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

			fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write);
		}

		//
		// performance metrics
		// 
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