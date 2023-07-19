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

typedef struct Tag_Color {
    u8 r;
    u8 g;
    u8 b;
} Color;

typedef struct Tag_Vertex {
    Vec3 position;
    Color color;
} Vertex;

typedef struct Tag_Projected_Vertex {
    Vec2I position;
    Color color;
} Projected_Vertex;

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

inline Vec2I ToPixelPositions(Vec2 ndc, int pixels_x, int pixels_y) {
    Vec2I result;
    result.x = (int)((ndc.x + 1.0f) / 2.0f * (float)pixels_x);
    result.y = (int)((ndc.y + 1.0f) / 2.0f * (float)pixels_y);
    return result;
}

inline b8 IsTopLeft(Vec2I edge) {
    return edge.y < 0 || (edge.x > 0 && edge.y == 0);
}
    
int EdgeCross(Vec2I minuend0, Vec2I minuend1, Vec2I subtrahend) {
    Vec2I difference0 = vec2i_sub(minuend0, subtrahend);
    Vec2I difference1 = vec2i_sub(minuend1, subtrahend);
    
    return difference0.x * difference1.y - difference0.y * difference1.x;
}

void RenderTriangleToBuffer(Offscreen_Buffer *buffer, Projected_Vertex v0, Projected_Vertex v1, Projected_Vertex v2) {
    int x_min = MAX(MIN(MIN(v0.position.x, v1.position.x), v2.position.x), 0);
    int y_min = MAX(MIN(MIN(v0.position.y, v1.position.y), v2.position.y), 0);
    int x_max = MIN(MAX(MAX(v0.position.x, v1.position.x), v2.position.x), buffer->width-1);
    int y_max = MIN(MAX(MAX(v0.position.y, v1.position.y), v2.position.y), buffer->height-1);

    int bias0 = IsTopLeft(vec2i_sub(v2.position, v1.position)) ? 0 : -1;
    int bias1 = IsTopLeft(vec2i_sub(v0.position, v2.position)) ? 0 : -1;
    int bias2 = IsTopLeft(vec2i_sub(v1.position, v0.position)) ? 0 : -1;
    
    int area = EdgeCross(v1.position, v2.position, v0.position);
    
    Vec2I p_start = { x_min, y_min };
    int w0 = EdgeCross(v2.position, p_start, v1.position) + bias0;
    int w1 = EdgeCross(v0.position, p_start, v2.position) + bias1;
    int w2 = EdgeCross(v1.position, p_start, v0.position) + bias2;
    
    int delta_w0_x = v1.position.y - v2.position.y;
    int delta_w1_x = v2.position.y - v0.position.y;
    int delta_w2_x = v0.position.y - v1.position.y;
    int delta_w0_y = (v2.position.x - v1.position.x) + ((x_min - x_max) * (v1.position.y - v2.position.y));
    int delta_w1_y = (v0.position.x - v2.position.x) + ((x_min - x_max) * (v2.position.y - v0.position.y));
    int delta_w2_y = (v1.position.x - v0.position.x) + ((x_min - x_max) * (v0.position.y - v1.position.y));
    
    u32 *pixel = (u32 *)buffer->memory;
    for (int y = y_min; y <= y_max; ++y) {
        for (int x = x_min; x <= x_max; ++x) {
            b8 inside_triangle = w0 >= 0 && w1 >= 0 && w2 >= 0;
            
            if (inside_triangle) {
                float alpha = (float)w0 / (float)area;
                float beta  = (float)w1 / (float)area;
                float gamma = (float)w2 / (float)area;
                
                u32 a = 0xFF;
                u32 r = (u32)(alpha * v0.color.r + beta * v1.color.r + gamma * v2.color.r);
                u32 g = (u32)(alpha * v0.color.g + beta * v1.color.g + gamma * v2.color.g);
                u32 b = (u32)(alpha * v0.color.b + beta * v1.color.b + gamma * v2.color.b);
                
                pixel[x + y * buffer->width] = a << 24 | r << 16 | g << 8 | b;
            }
            
            if (x == x_max) continue; // @note: this is needed because else the w_s are off by 1 * delta_w[0,1,2]_x for the rest of the loop
            w0 += delta_w0_x;
            w1 += delta_w1_x;
            w2 += delta_w2_x;
        }
        
        w0 += delta_w0_y;
        w1 += delta_w1_y;
        w2 += delta_w2_y;
    }
}

void RenderMeshToBuffer(Offscreen_Buffer *buffer, Projected_Vertex mesh[], u32 size) {
    Projected_Vertex triangle[3];

    triangle[0] = mesh[0]; // dont call RenderTriangleToBuffer with i == 0
    for (u32 i = 1; i < size; ++i) {
        if (i % 3 == 0) {
            RenderTriangleToBuffer(buffer, triangle[0], triangle[1], triangle[2]);
        }
        triangle[i % 3] = mesh[i];
    }
    RenderTriangleToBuffer(buffer, triangle[0], triangle[1], triangle[2]);
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
					
                    case VK_SPACE: {
                        process_key_event(SPACE, key_state, &event_reader);
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
    
    Projected_Vertex vertices0[] = {
        {{ 10, 20 }, { 255, 255, 0 }},
        {{ 90, 30 }, { 255, 128, 0 }},
        {{ 20, 100 }, { 255,   0, 0 }}
    };
    
    Projected_Vertex vertices1[] = {
        {{ 90, 30 }, { 0, 128, 255 }},
        {{ 80, 80 }, { 0, 255, 0 }},
        {{ 20, 100 }, { 255, 0, 255 }}
    };

    Vertex pyramid[] = {
        { {  0.0f, 0.0f,  1.0f }, {255, 255, 0} },
        { { -1.0f, 0.0f, -1.0f }, {255, 128, 0} },
        { {  1.0f, 0.0f, -1.0f }, {255, 0, 0} },
        
        { { -1.0f, 0.0f, -1.0f }, {0, 255, 0} },
        { {  0.0f, 2.0f,  0.0f }, {0, 0, 255} },
        { {  0.0f, 0.0f,  1.0f }, {255, 255, 0} },

        { { -1.0f, 0.0f, -1.0f }, {0, 255, 0} },
        { {  1.0f, 0.0f, -1.0f }, {255, 0, 0} },
        { {  0.0f, 2.0f,  0.0f }, {0, 0, 255} },

        { {  0.0f, 0.0f,  1.0f }, {255, 255, 0} },
        { {  0.0f, 2.0f,  0.0f }, {0, 0, 255} },
        { {  1.0f, 0.0f, -1.0f }, {255, 0, 0} }
    };
    
    float scale = 1.0f;
    Mat4 view  = mat4_identity();
    Mat4 proj  = ortho_projection(-WIDTH * scale, WIDTH * scale, HEIGHT * scale, -HEIGHT * scale, 0.1f, 100.0f);
    Mat4 view_proj = mat4_mul(view, proj);

    // @test
    float test = m_sin(4.5f);

    while (!global_should_close) {
        float delta_time = (float)frame_time / 1000.0f;
        
        platform_process_events();
        
        ClearFramebuffer(&global_backbuffer, 0x222222);
        
        Mat4 model = mat4_identity(); // @todo: rotate or something
        Mat4 mvp = mat4_mul(model, view_proj);

        Projected_Vertex mesh[12];
        
        for (int i = 0; i < SIZE(pyramid); ++i) {
            Vec4 pyramid4 = { pyramid[i].position.x, pyramid[i].position.y, pyramid[i].position.z, 1.0f };
            Vec4 result = mat4_vec4_mul(mvp, pyramid4);
            if (result.value.w != 0) {
                result.value.x /= result.value.w;
                result.value.y /= result.value.w;
                result.value.z /= result.value.w;
            }
            Vec2 ndc = { result.value.x, -result.value.y }; // mesh is flipped if y is not flipped
            mesh[i].position = ToPixelPositions(ndc, PIXELS_X, PIXELS_Y);
            mesh[i].color = pyramid[i].color;
        }

        RenderMeshToBuffer(&global_backbuffer, mesh, SIZE(mesh));
        CopyBufferToDisplay(&global_backbuffer, device_context, global_window.client_width,
                            global_window.client_height);
        
        /* if (!get_key_state(W).is_down) { */
        /*     RenderTriangleToBuffer(&global_backbuffer, vertices0[0], vertices0[1], vertices0[2]); */
        /* } */
        /* if (!get_key_state(S).is_down) { */
        /*     RenderTriangleToBuffer(&global_backbuffer, vertices1[0], vertices1[1], vertices1[2]); */
        /* } */
        /* CopyBufferToDisplay(&global_backbuffer, device_context, global_window.client_width, */
        /*                     global_window.client_height); */
        
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
