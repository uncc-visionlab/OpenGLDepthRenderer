#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>
#include <cxxopts.hpp>
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include "screenshots.hpp"
#include "YAML_Config.hpp"

#include <iostream>
#include <string>
#include <vector>


#define DEBUG 0

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
unsigned int SCR_WIDTH = 300;
unsigned int SCR_HEIGHT = 300;
float MAX_DEPTH = 1e5;
float zNear = 1.0e-2f;
float zFar = 10.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f);
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

glm::vec3 EuclideanToSpherical(const glm::vec3& euclid) {
    float r = std::sqrt(euclid.x * euclid.x + euclid.y * euclid.y + euclid.z * euclid.z);
    float theta = std::acos(euclid.y / r);
    float psi = std::atan2(euclid.z, euclid.x);
    return glm::vec3(r, theta, psi);
}

glm::vec3 SphericalToEuclidean(const glm::vec3& spherical) {
    const float& r = spherical.x;
    const float& theta = spherical.y;
    const float& psi = spherical.z;
    float x = r * std::cos(psi) * std::sin(theta);
    float z = r * std::sin(psi) * std::sin(theta);
    float y = r * std::cos(theta);
    return glm::vec3(x, y, z);
}

class DefaultScene {
    Model ourModel;
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    // plane VAO
    unsigned int planeVAO, planeVBO;
    unsigned int cubeTexture;
    unsigned int floorTexture;
public:

    DefaultScene() :
    ourModel("../../resources/objects/backpack/backpack.obj") {
        //Model ourModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
        //model("../../resources/objects/backpack/backpack.obj");
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
        cubeTexture = loadTexture(FileSystem::getPath("../../resources/textures/marble.jpg").c_str());
        floorTexture = loadTexture(FileSystem::getPath("../../resources/textures/metal.png").c_str());
    }

    ~DefaultScene() {
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteVertexArrays(1, &planeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteBuffers(1, &planeVBO);
    }

    void drawScene(Shader& shader) {
        // cubes
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glm::mat4 model(1.0f);
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
};

class VisibilityVolume {
public:
    int iWidth, iHeight;
    float fov_degrees;
    glm::vec3 origin, up, front;
    float up_max, up_min, radius_max;
    std::string output_filename;

    unsigned int numImages;
    unsigned int currentImageIndex;
    float *camera_thetas = nullptr;
    float *camera_phis = nullptr;
    GLfloat ** depth_imageArr = nullptr;

    VisibilityVolume() : iWidth(100), iHeight(100), fov_degrees(90),
    origin(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f), front(1.0f, 0.0f, 0.0f),
    output_filename("output.obj"), numImages(6), currentImageIndex(0) {
        up_max = std::numeric_limits<float>::max();
        radius_max = std::numeric_limits<float>::max();
        up_min = -std::numeric_limits<float>::max();
    }

    ~VisibilityVolume() {
        if (depth_imageArr != nullptr) {
            for (unsigned int i = 0; i < numImages; i++) {
                if (depth_imageArr[i] != nullptr) {
                    free(depth_imageArr[i]);
                }
            }
            free(depth_imageArr);
        }
    }

    void initializeWindowAndDepthBuffers(GLFWwindow* window) {
        // resize window and allow calls to resize framebuffer  
        if (iWidth != iHeight) {
            std::cout << "Visibility Volume Width and Height parameters must be equal." << std::endl;
            std::cout << "Forcing Width = Height = " << iHeight << "." << std::endl;
            iWidth = iHeight;
        }
        glfwSetWindowSize(window, iWidth, iHeight);
        // these calls are required per https://github.com/glfw/glfw/issues/1661
        glfwPollEvents();
        glfwWaitEvents();

        // initialize the depth buffers
        numImages = 6;
        currentImageIndex = 0;
        camera_thetas = (float *) realloc(camera_thetas, sizeof (float) * numImages);
        camera_phis = (float *) realloc(camera_phis, sizeof (float) * numImages);
        float phis[] = {0.0, 0.0f, 0.0f, 0.0f, 90.0f, -90.0f};
        float thetas[] = {0.0, 90.0f, 180.0f, 270.0f, 0.0f, 0.0f};
        for (unsigned int i = 0; i < numImages; i++) {
            camera_thetas[i] = thetas[i];
            camera_phis[i] = phis[i];
        }
        //glm::mat4 views[numImages];
        depth_imageArr = (GLfloat **) realloc(depth_imageArr, sizeof (GLfloat*) * numImages);
        for (unsigned int i = 0; i < numImages; i++) {
            //views[i] = glm::mat4(1.0f);
            depth_imageArr[i] = nullptr;
            depth_imageArr[i] = (GLfloat *) realloc(depth_imageArr[i], sizeof (GLfloat) * iWidth * iHeight);
        }
    }

    bool hasMoreImages() {
        return currentImageIndex < numImages;
    }

    glm::mat4 getNextCameraMatrix() {
        glm::mat4 viewMatrix(1.0f);
        if (currentImageIndex < numImages) {
            viewMatrix = getView(currentImageIndex);
        }
        return viewMatrix;
    }

    glm::mat4 getProjectionMatrix() {
        return MakeInfReversedZProjRH(glm::radians(fov_degrees), (float) iWidth / (float) iHeight, zNear);
    }

    void copyDepthBuffer() {
        glReadBuffer(GL_FRONT);
        //views[imageIdx] = view;
        glReadPixels(0, 0, iWidth, iHeight, GL_DEPTH_COMPONENT, GL_FLOAT, depth_imageArr[currentImageIndex]);
    }

#define toOBJIndex(i, j, k) (((iHeight * k + i) * iWidth) + j + 1)

    void writeVolumeToOBJ(YAML_CoordinateSystem world_coord_sys) {
        std::vector<glm::vec3> vertexCoordList;
        std::vector<glm::vec3> vertexColorList;
        std::vector<glm::u32vec3> vertexCoordIndexList;
        std::vector<glm::u32vec3> vertexColorIndexList;
        vertexColorList.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        vertexColorList.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 projection_inv = glm::inverse(getProjectionMatrix());
        if (DEBUG) {
            std::cout << "projection_inv = " << glm::to_string(projection_inv) << std::endl;
        }
        glm::mat4 view_inv;
        //FILE *f = fopen("ptcloud.data", "w");
        //float z_at_max_r;
        glm::vec4 pos_scr;
        glm::vec4 pos_3d;
        int i, j;
        unsigned int k, cur;
        for (k = 0; k < numImages; k++) {
            view_inv = glm::inverse(getView(k));
            if (DEBUG) {
                std::cout << "view_inv = " << glm::to_string(view_inv) << std::endl;
                std::cout << "determinant = " << glm::determinant(view_inv) << std::endl;
            }
            for (i = 0; i < iHeight; i++) {
                for (j = 0; j < iWidth; j++) {
                    cur = i * iWidth + j;
                    //cur = ((iHeight - i - 1) * iWidth + j);
                    //cur = ((iHeight - i - 1) * iWidth + j);
                    pos_scr.x = (float) 2.0f * (j + 0.5f) / iWidth - 1.0f;
                    pos_scr.y = (float) 2.0f * (i + 0.5f) / iHeight - 1.0f;
                    //pos.z = pixels[cur] * 2.0f - 1.0f;
                    pos_scr.z = depth_imageArr[k][cur];
                    pos_scr.w = 1.0f;
                    if (k == 4) {
                        //std::cout << "z = " << depth_imageArr[k][cur] << std::endl;
                    }

                    //if (std::abs(pos.z) < 1.0e-6f) { // max Z is 1/1.0e-7f
                    //    pos.z = (pos.z > 0) ? 1.0e-6f : -1.0e-6f;
                    //}
                    //if (std::sqrt(pos.x * pos.x + pos.y * pos.y + (1/pos.z)*(1/pos.z)) > MAX_RANGE) {
                    //if (std::abs(pos.z) < 1.0e-6f) { // max Z is 1/1.0e-7f
                    if (pos_scr.z < zNear / MAX_DEPTH) { // max Z is 1/1.0e-7f
                        //std::cout << "z = " << pos.z << " < zNear = " << zNear << std::endl;
                        //pos.z *= zNear / pos.z;
                        //pos.z = 0.001*zNear;
                        pos_scr.z = zNear / MAX_DEPTH;
                    }
                    if (std::abs(pos_scr.z) < 2.0f / (radius_max * radius_max)) { // max Z is 1/1.0e-7f
                        //z_at_max_r = 2.0f * sqrt(pos.x * pos.x + pos.y * pos.y + 1) / (MAX_RANGE * MAX_RANGE);
                        //pos.z = z_at_max_r;
                        //pos_scr.z = 2.0f * sqrt(pos_scr.x * pos_scr.x + pos_scr.y * pos_scr.y + 1) / (radius_max * radius_max);
                    }

                    pos_3d = view_inv * projection_inv * pos_scr;

                    pos_3d.x /= pos_3d.w;
                    pos_3d.y /= pos_3d.w;
                    pos_3d.z /= pos_3d.w;
                    pos_3d.w = 1.0f;

                    glm::vec3 euclidean_coords(pos_3d.x, pos_3d.y, pos_3d.z);

                    // restrict radius to radius_max
                    glm::mat4 world_coordinate_sys = world_coord_sys.getTransform();
                    //std::cout << "world_coords_xform = " << glm::to_string(world_coordinate_sys) << std::endl;
                    glm::vec4 world_coord4 = world_coordinate_sys * glm::vec4(euclidean_coords, 1.0f);
                    glm::vec3 world_coord3 = glm::vec3(world_coord4.x, world_coord4.y, world_coord4.z);
                    //glm::vec3 spherical_coords = EuclideanToSpherical(world_coord3);
                    glm::vec3 spherical_coords = EuclideanToSpherical(world_coord3 - origin);
                    if (spherical_coords.x > radius_max) {
                        spherical_coords.x = radius_max;
                        glm::vec3 euclidean_coords_new = SphericalToEuclidean(spherical_coords);
                        //euclidean_coords = euclidean_coords_new;
                        euclidean_coords = euclidean_coords_new + origin;
                        //std::cout << "radius_max exceeded: point = " << glm::to_string(euclidean_coords) << " ==> new point = " << glm::to_string(euclidean_coords_new) << std::endl;
                    }

                    // clamp height to the interval [up_min, up_max]
                    float height = glm::dot(euclidean_coords - world_coord_sys.origin, world_coord_sys.up);
                    if (height > up_max) {
                        euclidean_coords = euclidean_coords - (height - up_max) * up;
                    } else if (height < up_min) {
                        euclidean_coords = euclidean_coords + (up_min - height) * up;
                    }

                    if (i % 100 == 0 && j % 100 == 0 && DEBUG) {
                        //printf("%f %f %f\n", pos.x, pos.y, pos.z);
                        printf("%f %f %f\n", euclidean_coords.x, euclidean_coords.y, euclidean_coords.z);
                    }

                    vertexCoordList.push_back(euclidean_coords);
                    if (i > 0 && j > 0) {
                        int offsetA = iWidth * iHeight * k + (i - 1) * iWidth + j;
                        int offsetB = iWidth * iHeight * k + i * iWidth + j;
                        vertexCoordIndexList.push_back(glm::u32vec3(offsetA + 1, offsetA, offsetB));
                        vertexCoordIndexList.push_back(glm::u32vec3(offsetB, offsetB + 1, offsetA + 1));
                        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
                        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
                    }
                }
            }
        }
        // TODO: stitch 4 cylindrical seams of surfaces 
        // join the columns of image 0-3 with columns of image 1-4 respectively (where image 4 = image 0 closes the shape)
        int i_left, i_right, j_left, j_right, k_left, k_right;
        for (k = 0; k < 4; k++) {
            for (i = 1; i < iHeight; i++) {
                k_left = (k + 1) % 4;
                k_right = k;
                j_left = iWidth;
                j_right = 0 + 1;
                int offsetA = iWidth * iHeight * k_left + (i - 1) * iWidth + j_left;
                int offsetA_plus1 = iWidth * iHeight * k_right + (i - 1) * iWidth + j_right;
                int offsetB = iWidth * iHeight * k_left + i * iWidth + j_left;
                int offsetB_plus1 = iWidth * iHeight * k_right + i * iWidth + j_right;
                vertexCoordIndexList.push_back(glm::u32vec3(offsetA_plus1, offsetA, offsetB));
                vertexCoordIndexList.push_back(glm::u32vec3(offsetB, offsetB_plus1, offsetA_plus1));
                vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
                vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            }
        }
        // stitch bottom seams for the image with index k = 5
        k = 5;
        // join the columns of the top row of image 0 with columns of the bottom row of image 5
        for (j = 1; j < iWidth; j++) {
            k_left = 0;
            i_left = 0;
            j_left = j;
            k_right = k;
            i_right = iHeight - 1;
            j_right = j;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left + 1;
            int offsetB = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + i_right * iWidth + j_right + 1;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA, offsetA_plus1, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB_plus1, offsetB, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // join the columns of the top row of image 2 with the columns of the top row of image 5
        for (j = 1; j < iWidth; j++) {
            k_left = 2;
            i_left = 0;
            j_left = iWidth + 1 - j;
            k_right = k;
            i_right = 0;
            j_right = j;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left - 1;
            int offsetB = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + i_right * iWidth + j_right + 1;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA_plus1, offsetA, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB, offsetB_plus1, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // join the columns of the top row of image 3 with the rows of the last column of image 5
        for (i = 1; i < iHeight; i++) {
            k_left = 3;
            i_left = 0;
            j_left = i;
            k_right = k;
            i_right = iHeight - i;
            j_right = iWidth;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left + 1;
            int offsetB = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + (i_right - 1) * iWidth + j_right;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA, offsetA_plus1, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB_plus1, offsetB, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // join the columns of the top row of image 1 with the rows of the first column of image 5        
        for (i = 1; i < iHeight; i++) {
            k_left = 1;
            i_left = 0;
            j_left = i;
            k_right = k;
            i_right = i;
            j_right = 1;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left + 1;
            int offsetB = iWidth * iHeight * k_right + (i_right - 1) * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA, offsetA_plus1, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB_plus1, offsetB, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // insert 4 corner triangles that stitch 3 image corners together
        // k = 0, i = 0, j = 0
        // k = 5, i = iHeight - 1, j = 0
        // k = 1, i = 0, j = iWidth - 1
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(0, 0, 0),
                toOBJIndex(iHeight - 1, 0, 5),
                toOBJIndex(0, iWidth - 1, 1)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        // k = 1, i = 0, j = 0
        // k = 5, i = 0, j = 0
        // k = 2, i = 0, j = iWidth - 1
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(0, 0, 1),
                toOBJIndex(0, 0, 5),
                toOBJIndex(0, iWidth - 1, 2)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        // k = 2, i = 0, j = 0
        // k = 5, i = 0, j = iWidth - 1
        // k = 3, i = 0, j = iWidth - 1
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(0, 0, 2),
                toOBJIndex(0, iWidth - 1, 5),
                toOBJIndex(0, iWidth - 1, 3)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        // k = 3, i = 0, j = 0
        // k = 5, i = iHeight - 1, j = iWidth - 1
        // k = 0, i = 0, j = iWidth - 1
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(0, 0, 3),
                toOBJIndex(iHeight - 1, iWidth - 1, 5),
                toOBJIndex(0, iWidth - 1, 0)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));

        // stitch top seams for the image with index k = 4
        k = 4;
        // join the columns of the bottom row of image 0 with columns of the top row of image 4
        for (j = 1; j < iWidth; j++) {
            k_left = 0;
            i_left = iHeight - 1;
            j_left = j;
            k_right = k;
            i_right = 0;
            j_right = j;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left + 1;
            int offsetB = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + i_right * iWidth + j_right + 1;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA_plus1, offsetA, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB, offsetB_plus1, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // join the columns of the bottom row of image 2 with the columns of the bottom row of image 4
        for (j = 1; j < iWidth; j++) {
            k_left = 2;
            i_left = iHeight - 1;
            j_left = iWidth + 1 - j;
            k_right = k;
            i_right = iHeight - 1;
            j_right = j;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left - 1;
            int offsetB = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + i_right * iWidth + j_right + 1;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA, offsetA_plus1, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB_plus1, offsetB, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // join the columns of the bottom row of image 3 with the rows of the last column of image 4
        for (i = 1; i < iHeight; i++) {
            k_left = 3;
            i_left = iHeight - 1;
            j_left = i;
            k_right = k;
            i_right = i;
            j_right = iWidth;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left + 1;
            int offsetB = iWidth * iHeight * k_right + (i_right - 1) * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA_plus1, offsetA, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB, offsetB_plus1, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // join the columns of the bottom row of image 1 with the rows of the first column of image 4        
        for (i = 1; i < iHeight; i++) {
            k_left = 1;
            i_left = iHeight - 1;
            j_left = i;
            k_right = k;
            i_right = iHeight - i;
            j_right = 1;
            int offsetA = iWidth * iHeight * k_left + i_left * iWidth + j_left;
            int offsetA_plus1 = iWidth * iHeight * k_left + i_left * iWidth + j_left + 1;
            int offsetB = iWidth * iHeight * k_right + i_right * iWidth + j_right;
            int offsetB_plus1 = iWidth * iHeight * k_right + (i_right - 1) * iWidth + j_right;
            vertexCoordIndexList.push_back(glm::u32vec3(offsetA_plus1, offsetA, offsetB));
            vertexCoordIndexList.push_back(glm::u32vec3(offsetB, offsetB_plus1, offsetA_plus1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
            vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        }
        // insert 4 corner triangles that stitch 3 image corners together
        // k = 0, i = iHeight - 1, j = 0
        // k = 1, i = iHeight - 1, j = iWidth - 1
        // k = 4, i = 0, j = 0
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(iHeight - 1, 0, 0),
                toOBJIndex(iHeight - 1, iWidth - 1, 1), toOBJIndex(0, 0, 4)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        // k = 1, i = iHeight - 1, j = 0
        // k = 2, i = iHeight - 1, j = iWidth - 1
        // k = 4, i = 0, j = 0
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(iHeight - 1, 0, 1),
                toOBJIndex(iHeight - 1, iWidth - 1, 2), toOBJIndex(iHeight - 1, 0, 4)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        // k = 2, i = iHeight - 1, j = 0
        // k = 3, i = iHeight - 1, j = iWidth - 1
        // k = 4, i = iHeight - 1, j = iWidth - 1
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(iHeight - 1, 0, 2),
                toOBJIndex(iHeight - 1, iWidth - 1, 3), toOBJIndex(iHeight - 1, iWidth - 1, 4)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        // k = 3, i = iHeight - 1, j = 0
        // k = 0, i = iHeight - 1, j = iWidth - 1
        // k = 4, i = 0, j = iWidth - 1
        vertexCoordIndexList.push_back(glm::u32vec3(toOBJIndex(iHeight - 1, 0, 3),
                toOBJIndex(iHeight - 1, iWidth - 1, 0), toOBJIndex(0, iWidth - 1, 4)));
        vertexColorIndexList.push_back(glm::u32vec3(1, 1, 1));
        //fclose(f);

        FILE *fobj = fopen(output_filename.c_str(), "w");
        for (glm::vec3 vertex : vertexCoordList) {
            fprintf(fobj, "v %f %f %f\n", vertex.x, vertex.y, vertex.z);
        }
        for (glm::u32vec3 face : vertexCoordIndexList) {
            fprintf(fobj, "f %d %d %d\n", face.x, face.y, face.z);
        }
        fclose(fobj);
    }
private:

    glm::mat4 getView(int viewIndex) {
        if (camera_thetas[viewIndex] != 0 || camera_phis[viewIndex] != 0) {
            //if (true) {
            glm::vec4 other = glm::vec4(glm::cross(front, up), 1.0f);
            glm::mat4 rotateAzimuth = glm::rotate(glm::mat4(1.0f), glm::radians(camera_thetas[viewIndex]), up);
            glm::vec4 new_other = rotateAzimuth * other;
            glm::mat4 rotateToPhiTheta = glm::rotate(rotateAzimuth, glm::radians(camera_phis[viewIndex]), glm::vec3(new_other.x, new_other.y, new_other.z));
            glm::vec4 rotatedFront4 = rotateToPhiTheta * glm::vec4(front, 1.0f);
            glm::vec4 rotatedUp4 = rotateToPhiTheta * glm::vec4(up, 1.0f);
            glm::vec3 rotatedFront3 = glm::vec3(rotatedFront4.x, rotatedFront4.y, rotatedFront4.z);
            glm::vec3 rotatedUp3 = glm::vec3(rotatedUp4.x, rotatedUp4.y, rotatedUp4.z);
            return glm::lookAt(origin, origin + rotatedFront3, rotatedUp3);
        } else {
            return glm::lookAt(origin, origin + front, up);
        }
    }
};


// For details see: https://github.com/jarro2783/cxxopts

void cxxopts_integration(cxxopts::Options& options) {

    options.add_options()
            ("i,input", "Input file OBJ mesh format.", cxxopts::value<std::string>())
            ("x", "x Position of the camera", cxxopts::value<float>()->default_value("0"))
            ("y", "y Position of the camera", cxxopts::value<float>()->default_value("0"))
            ("z", "z Position of the camera", cxxopts::value<float>()->default_value("0"))
            ("rx", "x resolution of the camera in pixels", cxxopts::value<unsigned int>()->default_value("600"))
            ("ry", "y resolution of the camera in pixels", cxxopts::value<unsigned int>()->default_value("600"))
            ("c,config", "YAML config file.", cxxopts::value<std::string>())
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

    YAML_Config::YAML_ConfigPtr config_ptr;
    YAML_CoordinateSystem world_coord_sys;
    std::string configfile;
    if (result.count("config")) {
        configfile = result["config"].as<std::string>();
        config_ptr = std::make_shared<YAML_Config>();
        if (!config_ptr->parse(configfile)) {
            config_ptr = nullptr;
        }
        if (config_ptr != nullptr && config_ptr->coordsystems.size() > 0) {
            world_coord_sys = config_ptr->coordsystems[0];
            if (config_ptr->coordsystems.size() > 1) {
                std::cout << "Using first instance of world_coord_sys as the world coordinate system specification." << std::endl;
            }
        }
    }
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

    // configure global opengl state
    // -----------------------------
    // Reversed-Z means that far depth values are represented 
    // by smaller numbers. That means you need to switch your glDepthFunc 
    // from GL_LESS to GL_GREATER
    // https://dev.theomader.com/depth-precision/
    // https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

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

    std::vector<VisibilityVolume> visibility_vol_list;
    if (config_ptr != nullptr) {
        for (unsigned int i = 0; i < config_ptr->visibility_volumes.size(); i++) {
            VisibilityVolume vvol;
            vvol.iWidth = config_ptr->visibility_volumes[i].width;
            vvol.iHeight = config_ptr->visibility_volumes[i].height;
            vvol.fov_degrees = config_ptr->visibility_volumes[i].fov_degrees;
            vvol.origin = config_ptr->visibility_volumes[i].origin;
            vvol.front = config_ptr->visibility_volumes[i].front;
            vvol.up = config_ptr->visibility_volumes[i].up;
            vvol.up_max = config_ptr->visibility_volumes[i].up_max;
            vvol.up_min = config_ptr->visibility_volumes[i].up_min;
            vvol.radius_max = config_ptr->visibility_volumes[i].radius_max;
            vvol.output_filename = config_ptr->visibility_volumes[i].output_filename;
            visibility_vol_list.push_back(vvol);
        }
    } else {
        VisibilityVolume vvol;
        std::string outputfile = result["output"].as<std::string>();
        vvol.output_filename = outputfile;
        float target_x = result["x"].as<float>();
        float target_y = result["y"].as<float>();
        float target_z = result["z"].as<float>();
        vvol.origin = glm::vec3(target_x, target_y, target_z);
        float MAX_DEPTH = result["radius"].as<float>();
        vvol.radius_max = MAX_DEPTH;
        SCR_WIDTH = result["rx"].as<unsigned int>();
        SCR_HEIGHT = result["ry"].as<unsigned int>();
        vvol.iWidth = SCR_WIDTH;
        vvol.iHeight = SCR_HEIGHT;
        visibility_vol_list.push_back(vvol);
    }

    Model *loadedModel = NULL;
    DefaultScene *defaultScene;
    std::vector<Model> model_list;
    std::vector<glm::mat4> model_xforms;
    if (config_ptr != nullptr) {
        for (unsigned int i = 0; i < config_ptr->meshes.size(); i++) {
            inputfile = config_ptr->meshes[i].filename;
            loadedModel = new Model(inputfile);
            model_list.push_back(*loadedModel);
            model_xforms.push_back(config_ptr->meshes[i].getTransform());
            delete(loadedModel);
        }
    } else if (result.count("input")) {
        inputfile = result["input"].as<std::string>();
        loadedModel = new Model(inputfile);
        model_list.push_back(*loadedModel);
        model_xforms.push_back(glm::mat4(1.0f));
        delete(loadedModel);
    } else {
        std::cout << "No input file provided. Using the default scene" << std::endl;
        USE_BUILTIN_SCENE = true;
        defaultScene = new DefaultScene();
    }


    // build and compile and configure shaders
    // -------------------------
    Shader shader("depth_testing_revZ.vs", "depth_testing_revZ.fs");
    shader.use();
    shader.setInt("texture1", 0);

    // render loop
    glm::mat4 view, projection;

    // render
    // ------
    unsigned int vvol_index = 0;
    VisibilityVolume *vvol_ptr = nullptr;
    if (visibility_vol_list.size() > 0) {
        // initialize the visibility volume pointer 
        vvol_ptr = &visibility_vol_list[vvol_index];
        vvol_ptr->initializeWindowAndDepthBuffers(window);
    }
    //    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    //    glClearDepth(0.0f);
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        if (vvol_ptr != nullptr) { // we have visibility volumes to calculate
            // pointer has been initialized, test it to see if we need to increment the pointer and setup a new calculation
            if (!vvol_ptr->hasMoreImages()) {
                // write the calculated volume boundary surface as an OBJ file
                vvol_ptr->writeVolumeToOBJ(world_coord_sys);
                // go to the next visibility volume calculation 
                vvol_index++;
                if (vvol_index < visibility_vol_list.size()) {
                    vvol_ptr = &visibility_vol_list[vvol_index];
                    vvol_ptr->initializeWindowAndDepthBuffers(window);
                } else {
                    // finished processing visibility volume requests
                    vvol_ptr = nullptr;
                }
            }
        } else {
            // calculations are complete provide a user interface
            //processInput(window);
        }

        if (vvol_ptr != nullptr && vvol_ptr->hasMoreImages()) {
            view = vvol_ptr->getNextCameraMatrix();
            projection = vvol_ptr->getProjectionMatrix();
        } else {
            view = camera.GetViewMatrix();
            projection = MakeInfReversedZProjRH(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear);
        }

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glm::mat4 projection = glm::mat4(1.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);
        //projection = MakeInfReversedZProjRH(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);
        //glUniformMatrix4fv(GL_PROJECTION_MATRIX, 1, GL_FALSE, value_ptr(projection));
        //projection = perspectiveTransform(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);
        //projection = inversePerspectiveTransform(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, zNear, zFar);

        shader.setFloat("near", zNear);
        shader.setFloat("far", zFar);
        shader.setMat4("view", view);
        //std::cout << "view = " << glm::to_string(view) << std::endl;
        shader.setMat4("projection", projection);

        if (config_ptr != nullptr) {
            for (unsigned int i = 0; i < model_list.size(); i++) {
                shader.setMat4("model", model_xforms[i]);
                model_list[i].Draw(shader);
            }
            //        } else if (loadedModel != NULL) {
            //            model = glm::mat4(1.0f);
            //            shader.setMat4("model", model);
            //            loadedModel->Draw(shader);
        } else if (USE_BUILTIN_SCENE) {
            defaultScene->drawScene(shader);
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

        if (vvol_index >= visibility_vol_list.size()) {
            glfwSetWindowShouldClose(window, true);
            //glfwSetWindowSize(window, 640, 480);
        }

        if (vvol_ptr != nullptr) {
            vvol_ptr->copyDepthBuffer();
            vvol_ptr->currentImageIndex++;
        }
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

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
