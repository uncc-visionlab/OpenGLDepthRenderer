/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <png.h>

#include <stdlib.h>

GLfloat get_gl_depth(int x, int y) {
    float depth_z = 0.0f;

    glReadBuffer(GL_FRONT);
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth_z);
    return depth_z;
}


/* Take screenshot with glReadPixels and save to a file in PPM format.
 *
 * -   filename: file path to save to, without extension
 * -   width: screen width in pixels
 * -   height: screen height in pixels
 * -   pixels: intermediate buffer to avoid repeated mallocs across multiple calls.
 *     Contents of this buffer do not matter. May be NULL, in which case it is initialized.
 *     You must `free` it when you won't be calling this function anymore.
 */

void screenshot_float(const char *filename, unsigned int width,
        unsigned int height, GLfloat **pixels) {
    size_t i, j, cur;
    const size_t format_nchannels = 1;
    FILE *f = fopen(filename, "w");
    
    //fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
    *pixels = (GLfloat *) realloc(*pixels, format_nchannels * sizeof (GLfloat) * width * height);
    glReadBuffer(GL_FRONT);    
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, *pixels);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            cur = format_nchannels * ((height - i - 1) * width + j);
            //fprintf(f, "%3d %3d %3d ", (*pixels)[cur], (*pixels)[cur + 1], (*pixels)[cur + 2]);
            fprintf(f, "%f ", (*pixels)[cur]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}
        
void screenshot_float(const char *filename, unsigned int width, unsigned int height) {
    GLfloat *pixels = NULL;
    screenshot_float(filename, width, height, &pixels);
    free(pixels);
}

void screenshot_png(const char *filename, unsigned int width, unsigned int height,
        GLubyte **pixels, png_byte **png_bytes, png_byte ***png_rows) {
    size_t i, nvals;
    const size_t format_nchannels = 4;
    FILE *f = fopen(filename, "wb");
    nvals = format_nchannels * width * height;
    *pixels = (GLubyte*) realloc(*pixels, nvals * sizeof (GLubyte));
    *png_bytes = (png_byte*) realloc(*png_bytes, nvals * sizeof (png_byte));
    *png_rows = (png_byte**) realloc(*png_rows, height * sizeof (png_byte*));
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, *pixels);
    for (i = 0; i < nvals; i++)
        (*png_bytes)[i] = (*pixels)[i];
    for (i = 0; i < height; i++)
        (*png_rows)[height - i - 1] = &(*png_bytes)[i * width * format_nchannels];
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();
    png_infop info = png_create_info_struct(png);
    if (!info) abort();
    if (setjmp(png_jmpbuf(png))) abort();
    png_init_io(png, f);
    png_set_IHDR(
            png,
            info,
            width,
            height,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
            );
    png_write_info(png, info);
    png_write_image(png, *png_rows);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(f);
}

/* Adapted from https://github.com/cirosantilli/cpp-cheat/blob/19044698f91fefa9cb75328c44f7a487d336b541/png/open_manipulate_write.c */
void screenshot_png(const char *filename, unsigned int width, unsigned int height) {
    GLubyte *pixels = NULL;
    png_byte *png_bytes = NULL;
    png_byte **png_rows = NULL;
    screenshot_png(filename, width, height, &pixels, &png_bytes, &png_rows);
    free(pixels);
    free(png_bytes);
    free(png_rows);
}
