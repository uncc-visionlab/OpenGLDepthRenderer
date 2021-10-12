/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   screenshot_png.hpp
 * Author: arwillis
 *
 * Created on October 2, 2021, 3:12 PM
 */

#ifndef SCREENSHOT_PNG_HPP
#define SCREENSHOT_PNG_HPP

GLfloat get_gl_depth(int x, int y);
void screenshot_png(const char *filename, unsigned int width, unsigned int height);
void screenshot_float(const char *filename, unsigned int width, unsigned int height);

#endif /* SCREENSHOT_PNG_HPP */

