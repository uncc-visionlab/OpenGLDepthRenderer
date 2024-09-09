# OpenGLDepthRenderer
To run the code you will need an OpenGL 4.5 compatible graphics card as I use a couple of recent OpenGL tricks. Specifically, to obtain reasonable depth accuracy across the floating point numeric gamut I am using ```glClipControl()``` which requires OpenGL 4.5. To address floating point depth quantization issues as discussed in this article describing important [depth image precision considerations](https://developer.nvidia.com/content/depth-precision-visualized).

## Dependencies

The code also requires the following:

* [glfw library](https://www.glfw.org/) which can be installed as libglfw-dev on ubuntu machines.

* [libpng library](http://www.libpng.org/pub/png/libpng.html) which can be installed as libpng-dev on ubuntu machines.
* libxinerama-dev
* libxcursor-dev
* libxxf86m-dev

## Programs in this repository

A successful compile run will generate (4) targets:

1. OpenGLModelRenderer - this program renders a scene and will show textures on surfaces for a more realistic world

2. OpenGLDepthRenderer - this program renders only the scene depth as a grayscale, it will look strange and unrealistic

3. ogl_depthrenderer - this program implements inverse Z depth rendering which makes important accuracy improvements in the OpenGL depth values.

4. ogl_ML_data_augmenter - this program is part of a research effort that generates 3D models and simulates their sensed depth images, RGB images and the correct class labels for the purposes of training ML algorithms to recognize these objects.

The output of the ogl_depthrenderer is the OBJ format surface mesh.

## Compilation

### Linux
From the root of the source folder try the following command sequence in a console:
```
git submodule update --init
mkdir build
cd build
cmake ..
make
```
If the compile process succeeds without error your binaries will be in the ```build/bin``` subfolder.

## Interface

Note that the interface is barbarically simple. The window will steal the mouse focus. 

**IMPORTANT**

**To exit the program, Press the ESC key.**
