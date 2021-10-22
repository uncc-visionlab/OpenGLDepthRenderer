#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>
#include <cxxopts.hpp>
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include "screenshots.hpp"

#include <iostream>
#include <string>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
unsigned int SCR_WIDTH = 600;
unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = (float) SCR_WIDTH / 2.0;
float lastY = (float) SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// this is supposed to get the world position from the depth buffer

glm::mat4 invMakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear) {
    float f = 1.0f / tan(fovY_radians / 2.0f);
    return glm::mat4(
            aspectWbyH / f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f / zNear,
            0.0f, 0.0f, -1.0f, 0.0f);
}

glm::mat4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear) {
    float f = 1.0f / tan(fovY_radians / 2.0f);
    return glm::mat4(
            f / aspectWbyH, 0.0f, 0.0f, 0.0f,
            0.0f, f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, zNear, 0.0f);
}

glm::mat4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear, float zFar) {
    float f = 1.0f / tan(fovY_radians / 2.0f);
    return glm::mat4(
            f / aspectWbyH, 0.0f, 0.0f, 0.0f,
            0.0f, f, 0.0f, 0.0f,
            0.0f, 0.0f, zNear / (zNear - zFar), zNear * zFar / (zNear - zFar),
            0.0f, 0.0f, -1.0, 0.0f);
}

// a right-handed coordinate system with the negative z as gazing direction and the upward positive y direction. 

inline glm::mat4 perspectiveTransform(float fovY_radians, float aspectRatio, float n, float f) {

    float s = aspectRatio;
    float g = 1.0f / tan(fovY_radians / 2.0f); // focal length
    // A fine trick that i learned is that if you want to go back to the 
    // normal depth range, you can replace each value of n with f and 
    // vice-versa inside the expression for the constants A and B. Of 
    // course you also need to revert the other changes (GL_GEQUAL to 
    // GL_LEQUAL, clear to 1, comment out glClipControl).
    float A = n / (f - n);
    float B = f * n / (f - n);

    glm::mat4 forward = glm::mat4(g / s, 0.0f, 0.0f, 0.0f,
            0.0f, g, 0.0f, 0.0f,
            0.0f, 0.0f, A, B,
            0.0f, 0.0f, -1.0f, 0.0f);

    // Use this to test the correct mapping of near plane to 1.0f, and the
    // far plane to 0.0f
    glm::vec4 test0 = forward * glm::vec4(0.0f, 0.0f, -n, 1.0f);
    test0.x /= test0.w;
    test0.y /= test0.w;
    test0.z /= test0.w;
    glm::vec4 test1 = forward * glm::vec4(0.0f, 0.0f, -f, 1.0f);
    test1.x /= test1.w;
    test1.x /= test1.w;
    test1.x /= test1.w;

    return forward;
}

inline glm::mat4 inversePerspectiveTransform(float fovY_radians, float aspectRatio, float n, float f) {
    float s = aspectRatio;
    float g = 1.0f / tan(fovY_radians / 2.0f); // focal length
    // A fine trick that i learned is that if you want to go back to the 
    // normal depth range, you can replace each value of n with f and 
    // vice-versa inside the expression for the constants A and B. Of 
    // course you also need to revert the other changes (GL_GEQUAL to 
    // GL_LEQUAL, clear to 1, comment out glClipControl).
    float A = n / (f - n);
    float B = f * n / (f - n);
    // Precomputed inverse transform
    return glm::mat4(s / g, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / g, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, 1.0f / B, A / B);
}

// For details see: https://github.com/jarro2783/cxxopts

void cxxopts_integration(cxxopts::Options& options) {

    options.add_options()
            ("i,input", "Input file OBJ mesh format.", cxxopts::value<std::string>())
            ("x", "x Position of the camera", cxxopts::value<float>()->default_value("0"))
            ("y", "y Position of the camera", cxxopts::value<float>()->default_value("0"))
            ("z", "z Position of the camera", cxxopts::value<float>()->default_value("0"))
            ("rx", "x resolution of the camera in pixels", cxxopts::value<unsigned int>()->default_value("600"))
            ("ry", "y resolution of the camera in pixels", cxxopts::value<unsigned int>()->default_value("600"))
            ("r,radius", "Radius of visibility sphere", cxxopts::value<float>()->default_value("20"))
            ("o,output", "Output file <visibility_sphere.obj>", cxxopts::value<std::string>()->default_value("visibility_sphere.obj"))
            ("h,help", "Print usage")
            ;
}

int main(int argc, char **argv) {

    cxxopts::Options options("ogl_depthrenderer", "UNC Charlotte Machine Vision Lab Depth Rendering Code.");
    cxxopts_integration(options);
    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    std::string inputfile;
    bool USE_BUILTIN_SCENE = false;

    std::string outputfile = result["output"].as<std::string>();
    float target_x = result["x"].as<float>();
    float target_y = result["y"].as<float>();
    float target_z = result["z"].as<float>();
    camera.Position.x = target_x;
    camera.Position.y = target_y;
    camera.Position.z = target_z;
    float MAX_RANGE = result["radius"].as<float>();
    SCR_WIDTH = result["rx"].as<unsigned int>();
    SCR_HEIGHT = result["ry"].as<unsigned int>();
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGLDepthRenderer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Model *loadedModel = NULL;
    if (result.count("input")) {
        inputfile = result["input"].as<std::string>();
        loadedModel = new Model(inputfile);

    } else {
        std::cout << "No input file provided. Using the default scene" << std::endl;
        USE_BUILTIN_SCENE = true;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // implement reversed-Z opengl depth buffer
    // Reversed-Z in OpenGL
    // https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/ 
    // this command requires OpenGL 4.5 or higher
    // HWWS '99: Proceedings of the ACM SIGGRAPH/EUROGRAPHICS workshop on Graphics hardwareJuly 1999 Pages 67–73https://doi.org/10.1145/311534.311579

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if ((major > 4 || (major == 4 && minor >= 5))) {// || SDL_GL_ExtensionSupported("GL_ARB_clip_control")) {
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    } else {
        fprintf(stderr, "OpenGL 4.5 or higher required for glClipControl(). OpenGL %d.%d detected, sorry.\n", major, minor);
        exit(1);
    }
    // I use #define GLM_FORCE_DEPTH_ZERO_TO_ONE and the glm library for this

    // build and compile shaders
    // -------------------------
    Shader shader("depth_testing_revZ.vs", "depth_testing_revZ.fs");

    //Model ourModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
    Model ourModel("../../resources/objects/backpack/backpack.obj");
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };
    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
        5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
        -5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

        5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
        5.0f, -0.5f, -5.0f, 2.0f, 2.0f
    };
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*) (3 * sizeof (float)));
    glBindVertexArray(0);
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*) (3 * sizeof (float)));
    glBindVertexArray(0);

    // load textures
    // -------------
    unsigned int cubeTexture = loadTexture(FileSystem::getPath("../../resources/textures/marble.jpg").c_str());
    unsigned int floorTexture = loadTexture(FileSystem::getPath("../../resources/textures/metal.png").c_str());

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);

    // https://dev.theomader.com/depth-precision/
    // https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/

    // define a color buffer and a foating point depth buffer
    //    int width = 640, height = 480;
    //    GLuint color, depth, fbo;
    //
    //    glGenTextures(1, &color);
    //    glBindTexture(GL_TEXTURE_2D, color);
    //    glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, width, height);
    //    glBindTexture(GL_TEXTURE_2D, 0);
    //
    //    glGenTextures(1, &depth);
    //    glBindTexture(GL_TEXTURE_2D, depth);
    //    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
    //    glBindTexture(GL_TEXTURE_2D, 0);
    //
    //    glGenFramebuffers(1, &fbo);
    //    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    //    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //    if (status != GL_FRAMEBUFFER_COMPLETE) {
    //        fprintf(stderr, "glCheckFramebufferStatus: %x\n", status);
    //    }
    //    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //GLfloat depth = 0;
    float zNear = 0.1f;
    float zFar = 10.0f;
    glm::mat4 view, model, projection;
    // render loop

    int width, height;
    int numImages = 4;
    float rotationAngle_per_image = 2.0f * glm::pi<float>() / numImages;
    glfwGetWindowSize(window, &width, &height);

    GLfloat * cylindrical_proj_imageArr[numImages] = {NULL, NULL, NULL, NULL};
    glm::mat4 views[numImages];
    for (int i = 0; i < numImages; i++) {
        views[i] = glm::mat4(1.0f);
        cylindrical_proj_imageArr[i] = (GLfloat *) realloc(cylindrical_proj_imageArr[i], sizeof (GLfloat) * width * height);
    }

    //camera.Zoom = glm::pi<float>() / 2.0f;
    camera.Up = glm::vec3(0.0f, 0.0f, 1.0f);
    camera.Front = glm::vec3(1.0f, 0.0f, 0.0f);
    camera.Zoom = 90.0f;
    int imageIdx = 0;
    glm::vec3 frontVec = camera.Front;

    std::vector<glm::vec4> vertexCoordList;
    std::vector<glm::vec3> vertexColorList;
    std::vector<glm::u32vec3> vertexCoordIndexList;
    std::vector<glm::u32vec3> vertexColorIndexList;
    vertexColorList.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    vertexColorList.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        if (imageIdx >= numImages) {
            processInput(window);
        }

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Reversed-Z means that far depth values are represented 
        // by smaller numbers. That means you need to switch your glDepthFunc 
        // from GL_LESS to GL_GREATER
        glDepthFunc(GL_GREATER);
        //glEnable(GL_DEPTH_TEST);

        shader.use();
        model = glm::mat4(1.0f);
        //glm::mat4 view = glm::mat4(1.0f);
        //glm::mat4 view = camera.GetViewMatrix();
        if (imageIdx < numImages) {
            glm::mat4 rotateMat4 = glm::rotate(glm::mat4(1.0f), imageIdx * rotationAngle_per_image, camera.Up);
            glm::vec4 rotatedFront = rotateMat4 * glm::vec4(frontVec, 1.0f);
            camera.Front.x = rotatedFront.x;
            camera.Front.y = rotatedFront.y;
            camera.Front.z = rotatedFront.z;
        }
        glm::mat4 view = camera.GetViewMatrix();

        //glm::mat4 projection = glm::mat4(1.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);
        projection = MakeInfReversedZProjRH(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear);
        //projection = MakeInfReversedZProjRH(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);
        //glUniformMatrix4fv(GL_PROJECTION_MATRIX, 1, GL_FALSE, value_ptr(projection));
        //projection = perspectiveTransform(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);
        //projection = inversePerspectiveTransform(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);

        shader.setFloat("near", zNear);
        shader.setFloat("far", zFar);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        if (loadedModel != NULL) {
            model = glm::mat4(1.0f);
            shader.setMat4("model", model);
            ourModel.Draw(shader);
        } else if (USE_BUILTIN_SCENE) {
            // cubes
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTexture);
            model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -4.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 0.0f, -3.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // floor
            glBindVertexArray(planeVAO);

            glBindTexture(GL_TEXTURE_2D, floorTexture);
            shader.setMat4("model", glm::mat4(1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 10.0f));
            shader.setMat4("model", model);
            ourModel.Draw(shader);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
            //model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            shader.setMat4("model", model);
            ourModel.Draw(shader);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            shader.setMat4("model", model);
            ourModel.Draw(shader);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            shader.setMat4("model", model);
            ourModel.Draw(shader);
        }
        // reset the comparison and depth state back to OpenGL defaults, 
        // so the state doesn’t leak into other code that might not be doing 
        // Reversed-Z, or that might not be using depth testing
        //glDepthFunc(GL_LESS);
        //glDisable(GL_DEPTH_TEST);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        //glfwSetWindowShouldClose(window, true);

        if (imageIdx < numImages) {
            glReadBuffer(GL_FRONT);
            views[imageIdx] = view;
            glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, cylindrical_proj_imageArr[imageIdx++]);
        }
    }
    delete(loadedModel);

    glm::mat4 projection_inv = glm::inverse(projection);
    //FILE *f = fopen("ptcloud.data", "w");
    glm::f32vec4 pos;
    int i, j, k, cur;
    for (i = 0; i < height; i++) {
        for (k = 0; k < numImages; k++) {
            glm::mat4 view_inv = glm::inverse(views[k]);
            for (j = 0; j < width; j++) {
                cur = ((height - i - 1) * width + j);
                pos.x = (float) 2.0f * j / width - 1.0f;
                pos.y = (float) 2.0f * i / height - 1.0f;
                //pos.z = pixels[cur] * 2.0f - 1.0f;
                pos.z = cylindrical_proj_imageArr[k][cur];
                if (pos.z < 1.0e-6f) { // max Z is 1/1.0e-6f
                    pos.z = 2.0f * sqrt(pos.x * pos.x + pos.y * pos.y + 1) / (MAX_RANGE * MAX_RANGE);
                }
                pos.w = 1.0f;

                glm::f32vec4 pos3d = view_inv * projection_inv * pos;
                pos3d.x /= pos3d.w;
                pos3d.y /= pos3d.w;
                pos3d.z /= pos3d.w;
                if (i % 100 == 0 && j % 100 == 0) {
                    //printf("%f %f %f\n", pos.x, pos.y, pos.z);
                    printf("%f %f %f\n", pos3d.x, pos3d.y, pos3d.z);
                }
                //fprintf(f, "%f %f %f\n", pos3d.x, pos3d.y, pos3d.z);

                vertexCoordList.push_back(pos3d);
                if (i > 0 && j > 0) {
                    int offsetA = (i - 1) * width * numImages + j + k * width;
                    int offsetB = i * width * numImages + j + k * width;
                    vertexCoordIndexList.push_back(glm::u32vec3(offsetA, offsetA + 1, offsetB));
                    vertexCoordIndexList.push_back(glm::u32vec3(offsetB, offsetA + 1, offsetB + 1));
                    vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
                }
            }
        }
    }
    //fclose(f);


    FILE *fobj = fopen(outputfile.c_str(), "w");
    for (glm::vec4 vertex : vertexCoordList) {
        fprintf(fobj, "v %f %f %f\n", vertex.x, vertex.y, vertex.z);
    }
    for (glm::u32vec3 face : vertexCoordIndexList) {
        fprintf(fobj, "f %d %d %d\n", face.x, face.y, face.z);
    }
    fclose(fobj);

    //int width, height;
    glfwGetWindowSize(window, &width, &height);
    //snprintf(filename, SCREENSHOT_MAX_FILENAME, "tmp.%d.png", nframes);
    screenshot_png("depth_screenshot.png", width, height);
    screenshot_float("depth_float_screenshot.png", width, height);

    //glm::mat4 model = glm::mat4(1.0f);
    //glm::mat4 view = camera.GetViewMatrix();
    //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);

    GLfloat *pixels = NULL;
    pixels = (GLfloat *) realloc(pixels, sizeof (GLfloat) * width * height);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);

    free(pixels);

    for (int i = 0; i < numImages; i++) {
        free(cylindrical_proj_imageArr[i]);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

void processInput(GLFWwindow * window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(-yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------

unsigned int loadTexture(char const *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
