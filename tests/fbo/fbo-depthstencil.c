/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/** @file fbo-depthstencil.c
 *
 * Tests glClear, glReadPixels, glDrawPixels, glCopyPixels, glBlitFramebuffer
 * with depth-stencil buffers.
 */

#include "piglit-util-gl.h"

#define BUF_SIZE 123

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = BUF_SIZE;
	config.window_height = BUF_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum {
	CLEAR,
	READPIXELS,
	DRAWPIXELS,
	COPYPIXELS,
	BLIT
};
int test = CLEAR;

#define F(name) #name, name

struct format {
	const char *name;
	GLenum iformat;
	const char *extension;
} formats[] = {
	{"default_fb", 0, NULL},
	{F(GL_DEPTH24_STENCIL8),  "GL_EXT_packed_depth_stencil"},
	{F(GL_DEPTH32F_STENCIL8), "GL_ARB_depth_buffer_float"}
};

struct packformat {
	const char *name;
	GLenum type;
	const char *extension;
} packformats[] = {
	{"FLOAT-and-USHORT", 0, NULL},
	{"24_8", GL_UNSIGNED_INT_24_8, "GL_EXT_packed_depth_stencil"},
	{"32F_24_8_REV", GL_FLOAT_32_UNSIGNED_INT_24_8_REV, "GL_ARB_depth_buffer_float"}
};

struct format f;
struct packformat pf;
GLuint stencilmask;

static enum piglit_result test_clear(void)
{
	GLuint cb;
	GLenum status;
	float green[3] = {0, 1, 0};
	enum piglit_result res;

	/* Add a colorbuffer. */
	if (f.iformat) {
		glGenRenderbuffersEXT(1, &cb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, cb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, BUF_SIZE, BUF_SIZE);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0,
					     GL_RENDERBUFFER_EXT, cb);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("FBO incomplete status 0x%X\n", status);
			piglit_report_result(PIGLIT_FAIL); /* RGBA8 must succeed. */
		}
	}

	glClearDepth(0.75);
	glClearStencil(0x3456);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x3456 & stencilmask, ~0);

	glColor3fv(green);
	piglit_draw_rect_z(0.5, -1, -1, 2, 2); /* 0.75 converted to clip space is 0.5. */
	glColor3f(1, 1, 1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	res = piglit_probe_rect_rgb(0, 0, BUF_SIZE, BUF_SIZE, green) ? PIGLIT_PASS : PIGLIT_FAIL;

	/* Display the colorbuffer. */
	if (!piglit_automatic && f.iformat) {
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glBlitFramebufferEXT(0, 0, BUF_SIZE, BUF_SIZE, 0, 0, BUF_SIZE, BUF_SIZE,
				     GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	if (f.iformat)
		glDeleteRenderbuffersEXT(1, &cb);

	return res;
}

typedef void (*transfer_func)(GLfloat*, GLushort*);

static void read_separately(GLfloat *depth, GLushort *stencil)
{
	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_DEPTH_COMPONENT,
		     GL_FLOAT, depth);
	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_STENCIL_INDEX,
		     GL_UNSIGNED_SHORT, stencil);
}

static void read_24_8(GLfloat *depth, GLushort *stencil)
{
	GLuint buf[BUF_SIZE*BUF_SIZE];
	int x, y;

	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_DEPTH_STENCIL,
		     GL_UNSIGNED_INT_24_8, buf);

	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			depth[y*BUF_SIZE+x] = (buf[y*BUF_SIZE+x] >> 8) / (float)0xffffff;
			stencil[y*BUF_SIZE+x] = buf[y*BUF_SIZE+x] & 0xff;
		}
	}
}

static void read_32f_8(GLfloat *depth, GLushort *stencil)
{
	GLuint buf[BUF_SIZE*BUF_SIZE*2];
	float *buff = (float*)buf;
	int x, y;

	glReadPixels(0, 0, BUF_SIZE, BUF_SIZE, GL_DEPTH_STENCIL,
		     GL_FLOAT_32_UNSIGNED_INT_24_8_REV, buf);

	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			depth[y*BUF_SIZE+x] = buff[(y*BUF_SIZE+x)*2];
			stencil[y*BUF_SIZE+x] = buf[(y*BUF_SIZE+x)*2+1] & 0xff;
		}
	}
}

static enum piglit_result compare(transfer_func read)
{
	int x, y, failures = 0;
	GLfloat depth[BUF_SIZE*BUF_SIZE];
	GLfloat expected_depth;
	GLushort stencil[BUF_SIZE*BUF_SIZE];
	GLushort expected_stencil;

	/* Read buffers. */
	read(depth, stencil);

	/* Compare results. */
	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {

			/* Skip the middle row and column of pixels because
			 * drawing polygons for the left/right and bottom/top
			 * quadrants may hit the middle pixels differently
			 * depending on minor transformation and rasterization
			 * differences.
			 */
			if (x == BUF_SIZE / 2 || y == BUF_SIZE / 2)
				continue;

			if (y < BUF_SIZE/2) {
				expected_depth = x < BUF_SIZE/2 ? 0.25 : 0.375;
				expected_stencil = (x < BUF_SIZE/2 ? 0x3333 : 0x6666) & stencilmask;
			} else {
				expected_depth = x < BUF_SIZE/2 ? 0.625 : 0.75;
				expected_stencil = (x < BUF_SIZE/2 ? 0x9999 : 0xbbbb) & stencilmask;
			}

			if (fabs(depth[y*BUF_SIZE+x] - expected_depth) > 0.001) {
				failures++;
				if (failures < 20) {
					printf("Depth at %i,%i   Expected: %f   Observed: %f\n",
						x, y, expected_depth, depth[y*BUF_SIZE+x]);
				} else if (failures == 20) {
					printf("...\n");
				}
			}
			if (stencil[y*BUF_SIZE+x] != expected_stencil) {
				failures++;
				if (failures < 20) {
					printf("Stencil at %i,%i   Expected: 0x%02x   Observed: 0x%02x\n",
						x, y, expected_stencil, stencil[y*BUF_SIZE+x]);
				} else if (failures == 20) {
					printf("...\n");
				}
			}
		}
	}
	if (failures)
		printf("Total failures: %i\n", failures);

	return failures == 0 ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result test_readpixels(transfer_func read)
{
	/* Clear. */
	glClearDepth(0);
	glClearStencil(0xfefe);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Initialize. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glStencilFunc(GL_ALWAYS, 0x3333 & stencilmask, ~0);
	piglit_draw_rect_z(-0.5, -1, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x6666 & stencilmask, ~0);
	piglit_draw_rect_z(-0.25, 0, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x9999 & stencilmask, ~0);
	piglit_draw_rect_z(0.25, -1, 0, 1, 1);

	glStencilFunc(GL_ALWAYS, 0xbbbb & stencilmask, ~0);
	piglit_draw_rect_z(0.5, 0, 0, 1, 1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	return compare(read);
}

static void draw_separately(GLfloat *depth, GLushort *stencil)
{
	glDrawPixels(BUF_SIZE, BUF_SIZE, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
	glDrawPixels(BUF_SIZE, BUF_SIZE, GL_STENCIL_INDEX, GL_UNSIGNED_SHORT, stencil);
}

static void draw_24_8(GLfloat *depth, GLushort *stencil)
{
	GLuint buf[BUF_SIZE*BUF_SIZE];
	int x, y;

	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			buf[y*BUF_SIZE+x] =
				(stencil[y*BUF_SIZE+x] & 0xff) |
				((GLuint)(depth[y*BUF_SIZE+x] * (double)0xffffff) << 8);
		}
	}

	glDrawPixels(BUF_SIZE, BUF_SIZE, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, buf);
}

static void draw_32f_8(GLfloat *depth, GLushort *stencil)
{
	GLuint buf[BUF_SIZE*BUF_SIZE*2];
	GLfloat *buff = (GLfloat*)buf;
	int x, y;

	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			buff[(y*BUF_SIZE+x)*2] = depth[y*BUF_SIZE+x];
			buf[(y*BUF_SIZE+x)*2+1] = stencil[y*BUF_SIZE+x] & 0xff;
		}
	}

	glDrawPixels(BUF_SIZE, BUF_SIZE, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, buf);
}

static enum piglit_result test_drawpixels(transfer_func draw, transfer_func read)
{
	int x, y;
	GLfloat depth[BUF_SIZE*BUF_SIZE];
	GLushort stencil[BUF_SIZE*BUF_SIZE];

	for (y = 0; y < BUF_SIZE; y++) {
		for (x = 0; x < BUF_SIZE; x++) {
			if (y < BUF_SIZE/2) {
				depth[y*BUF_SIZE+x] = x < BUF_SIZE/2 ? 0.25 : 0.375;
				stencil[y*BUF_SIZE+x] = (x < BUF_SIZE/2 ? ~0x3333 : ~0x6666) & stencilmask;
			} else {
				depth[y*BUF_SIZE+x] = x < BUF_SIZE/2 ? 0.625 : 0.75;
				stencil[y*BUF_SIZE+x] = (x < BUF_SIZE/2 ? ~0x9999 : ~0xbbbb) & stencilmask;
			}
		}
	}

	/* Clear. */
	glClearDepth(0);
	glClearStencil(0xfefe);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Draw pixels. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	draw(depth, stencil);
	glDisable(GL_DEPTH_TEST);

	/* Invert bits. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
	piglit_draw_rect(-1, -1, 2, 2);
	glDisable(GL_STENCIL_TEST);

	return compare(read);
}

static enum piglit_result test_copy(void)
{
	/* Clear. */
	glClearDepth(0);
	glClearStencil(0xfefe);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* Initialize buffers. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	/* Set the upper-right corner to 0x3333 and copy the content to the lower-left one. */
	glStencilFunc(GL_ALWAYS, 0x3333 & stencilmask, ~0);
	piglit_draw_rect_z(-0.5, 0, 0, 1, 1);
	if (test == BLIT)
		glBlitFramebufferEXT(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE, BUF_SIZE,
				     0, 0, BUF_SIZE/2, BUF_SIZE/2,
				     GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	else
		glCopyPixels(BUF_SIZE/2+1, BUF_SIZE/2+1, BUF_SIZE/2, BUF_SIZE/2, GL_DEPTH_STENCIL);

	/* Initialize the other corners. */
	glStencilFunc(GL_ALWAYS, 0x6666 & stencilmask, ~0);
	piglit_draw_rect_z(-0.25, 0, -1, 1, 1);

	glStencilFunc(GL_ALWAYS, 0x9999 & stencilmask, ~0);
	piglit_draw_rect_z(0.25, -1, 0, 1, 1);

	glStencilFunc(GL_ALWAYS, 0xbbbb & stencilmask, ~0);
	piglit_draw_rect_z(0.5, 0, 0, 1, 1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	return compare(read_separately);
}

enum piglit_result piglit_display(void)
{
	enum piglit_result res;
	GLuint fb, rb;
	GLint stencil_size;
	GLenum status;
	transfer_func read = NULL, draw = NULL;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Create the FBO. */
	if (f.iformat) {
		glGenRenderbuffersEXT(1, &rb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, f.iformat, BUF_SIZE, BUF_SIZE);
		glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_STENCIL_SIZE_EXT, &stencil_size);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		glGenFramebuffersEXT(1, &fb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT,
					GL_RENDERBUFFER_EXT, rb);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT,
					GL_RENDERBUFFER_EXT, rb);
		glViewport(0, 0, BUF_SIZE, BUF_SIZE);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("FBO incomplete status 0x%X\n", status);
			piglit_report_result(PIGLIT_SKIP);
		}
	} else {
		stencil_size = 8;
	}

	stencilmask = (1 << stencil_size) - 1;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	switch (pf.type) {
	case 0:
		read = read_separately;
		draw = draw_separately;
		break;
	case GL_UNSIGNED_INT_24_8:
		read = read_24_8;
		draw = draw_24_8;
		break;
	case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
		read = read_32f_8;
		draw = draw_32f_8;
		break;
	default:
		assert(0);
	}

	switch (test) {
	case CLEAR:
		puts("Testing glClear(depthstencil).");
		res = test_clear();
		break;
	case READPIXELS:
		puts("Testing glReadPixels(depthstencil).");
		res = test_readpixels(read);
		break;
	case DRAWPIXELS:
		puts("Testing glDrawPixels(depthstencil).");
		res = test_drawpixels(draw, read);
		break;
	case COPYPIXELS:
	case BLIT:
		puts(test == BLIT ?
		     "Testing glBlitFramebuffer(depthstencil)." :
		     "Testing glCopyPixels(depthstencil).");
		res = test_copy();
		break;
	default:
		assert(0);
		res = PIGLIT_SKIP;
	}

	/* Cleanup. */
	if (f.iformat) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glDeleteFramebuffersEXT(1, &fb);
		glDeleteRenderbuffersEXT(1, &rb);
	}

	piglit_present_results();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	return res;
}

void piglit_init(int argc, char **argv)
{
	unsigned i, p;

	for (p = 1; p < argc; p++) {
		if (!strcmp(argv[p], "clear")) {
			test = CLEAR;
			continue;
		}
		if (!strcmp(argv[p], "readpixels")) {
			test = READPIXELS;
			continue;
		}
		if (!strcmp(argv[p], "drawpixels")) {
			test = DRAWPIXELS;
			continue;
		}
		if (!strcmp(argv[p], "copypixels")) {
			test = COPYPIXELS;
			continue;
		}
		if (!strcmp(argv[p], "blit")) {
			test = BLIT;
			piglit_require_extension("GL_EXT_framebuffer_object");
			piglit_require_extension("GL_EXT_framebuffer_blit");
			continue;
		}
		for (i = 0; i < sizeof(formats)/sizeof(*formats); i++) {
			if (!strcmp(argv[p], formats[i].name)) {
				if (formats[i].extension)
					piglit_require_extension(formats[i].extension);
				f = formats[i];
				printf("Testing %s.\n", f.name);
				break;
			}
		}
		for (i = 0; i < sizeof(packformats)/sizeof(*packformats); i++) {
			if (!strcmp(argv[p], packformats[i].name)) {
				if (packformats[i].extension)
					piglit_require_extension(packformats[i].extension);
				pf = packformats[i];
				printf("Testing %s.\n", pf.name);
				break;
			}
		}
	}

	if (!f.name) {
		printf("Not enough parameters.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (f.iformat) {
	        piglit_require_extension("GL_EXT_framebuffer_object");
		piglit_require_extension("GL_EXT_framebuffer_blit");
	}
}
