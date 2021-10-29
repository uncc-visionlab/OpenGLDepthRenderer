/*
 * Copyright (C) 2021 Andrew R. Willis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef YAML_CONFIG_H
#define YAML_CONFIG_H

// YAML config file support
#include <yaml-cpp/yaml.h>

#include <glm/glm.hpp>

// std includes
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

class YAML_Object {
public:

    YAML_Object() {
    }
    bool parse(const YAML::Node& object);

    std::string id;
};

class YAML_CoordinateSystem : public YAML_Object {
public:

    YAML_CoordinateSystem() : origin(0.0f), up(0.0f, 1.0f, 0.0f), front(1.0f, 0.0f, 0.0f) {
    }
    bool parse(const YAML::Node& camera);
    //Camera::CameraPtr makeCamera();

    glm::vec3 origin;
    glm::vec3 up;
    glm::vec3 front;
};

class YAML_VisibilityVolume : public YAML_Object {
public:

    YAML_VisibilityVolume() : width(0), height(0), up(0.0f, 1.0f, 0.0f), front(1.0f, 0.0f, 0.0f) {
        max_altitude = std::numeric_limits<float>::max();
        min_altitude = std::numeric_limits<float>::min();
    }
    bool parse(const YAML::Node& camera);
    //Camera::CameraPtr makeCamera();

    int width;
    int height;
    glm::vec3 origin;
    glm::vec3 up;
    glm::vec3 front;
    float fov_degrees;
    float max_altitude;
    float min_altitude;
};

class YAML_Object3D : public YAML_Object {
public:

    YAML_Object3D() {
        scale = glm::vec3(1.0f);
        position = glm::vec3(0.0f);
        orientation_axis_angle = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    }
    bool parse(const YAML::Node& camera);
    glm::mat4 getTransform();

    glm::vec3 position;
    glm::vec4 orientation_axis_angle;
    glm::vec3 scale;
};

class YAML_Mesh : public YAML_Object3D {
public:

    YAML_Mesh() : material("Default") {
    }
    bool parse(const YAML::Node& mesh);
    //    std::vector<tinyobj::shape_t> getShapes();
    //Mesh getMesh();

    std::string format;
    std::string filename;
    std::string material;
    std::string geometry_type;
};

class YAML_Config {
public:
    typedef std::shared_ptr<YAML_Config> YAML_ConfigPtr;


    //YAML_CoordinateSystem camera;
    std::vector<YAML_CoordinateSystem> coordsystems;
    std::vector<YAML_Mesh> meshes;
    std::vector<YAML_VisibilityVolume> visibility_volumes;
    bool parse(const std::string);
};

#endif /* YAML_CONFIG_H */

