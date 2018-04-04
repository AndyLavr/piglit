/*
 * vkrunner
 *
 * Copyright (C) 2013, 2014 Neil Roberts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "vr-buffer.h"
#include "piglit-util.h"

void
vr_buffer_init(struct vr_buffer *buffer)
{
	static const struct vr_buffer init = VR_BUFFER_STATIC_INIT;

	*buffer = init;
}

void
vr_buffer_ensure_size(struct vr_buffer *buffer,
		      size_t size)
{
	size_t new_size = MAX2(buffer->size, 1);

	while (new_size < size)
		new_size *= 2;

	if (new_size != buffer->size) {
		buffer->data = realloc(buffer->data, new_size);
		buffer->size = new_size;
	}
}

void
vr_buffer_set_length(struct vr_buffer *buffer,
		     size_t length)
{
	vr_buffer_ensure_size(buffer, length);
	buffer->length = length;
}

void
vr_buffer_append(struct vr_buffer *buffer,
		 const void *data,
		 size_t length)
{
	vr_buffer_ensure_size(buffer, buffer->length + length);
	memcpy(buffer->data + buffer->length, data, length);
	buffer->length += length;
}

void
vr_buffer_append_string(struct vr_buffer *buffer,
			const char *str)
{
	vr_buffer_append(buffer, str, strlen(str) + 1);
	buffer->length--;
}

void
vr_buffer_destroy(struct vr_buffer *buffer)
{
	free(buffer->data);
}
