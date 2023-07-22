#ifndef LOGGER_H_
#define LOGGER_H_

#include <windows.h>

#include <stdio.h>

#define MAX_MESSAGE_LENGTH 256
void *output_handle = 0;
char out_message[MAX_MESSAGE_LENGTH] = {0};

void platform_logging_init() {
	AllocConsole();
	output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
}

void platform_log(const char *message, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, message);

	vsnprintf(out_message, MAX_MESSAGE_LENGTH, message, arg_ptr);

	va_end(arg_ptr);

	if (output_handle == 0) return;
	WriteConsole(output_handle, out_message, MAX_MESSAGE_LENGTH - 1, 0, 0);
}

void platform_logging_free() {
	output_handle = 0;
	FreeConsole();
}

#endif
