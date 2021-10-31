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

#include "YAML_Config.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

// YAML_Object

bool YAML_Object::parse(const YAML::Node& object) {
    if (object) {
        if (object["id"]) {
            id = object["id"].as<std::string>();
        } else {
            std::cout << "Error: object missing required id value." << std::endl;
            return false;
        }
    }
    return true;
}

// YAML_Config

bool YAML_Config::parse(const std::string filenameYAMLConfig) {
    YAML::Node doc = YAML::LoadFile(filenameYAMLConfig);

    std::vector<YAML_VisibilityVolume>& visibility_volumes_yaml = visibility_volumes;
    std::vector<YAML_Mesh>& meshes_yaml = meshes;
    std::vector<YAML_CoordinateSystem>& coordsystems_yaml = coordsystems;

    for (YAML::iterator it = doc.begin(); it != doc.end(); ++it) {
        std::cout << it->first << std::endl;
        std::cout << it->second << std::endl;
        if (it->first.Scalar() == "visibility_vol") {
            const YAML::Node& visibility_node = it->second;
            YAML_VisibilityVolume visibility_vol_yaml;
            if (!visibility_vol_yaml.parse(visibility_node)) {
                return false;
            }
            visibility_volumes_yaml.push_back(visibility_vol_yaml);
        } else if (it->first.Scalar() == "mesh") {
            const YAML::Node& mesh_node = it->second;
            YAML_Mesh mesh_yaml;
            if (!mesh_yaml.parse(mesh_node)) {
                return false;
            }
            meshes_yaml.push_back(mesh_yaml);
        } else if (it->first.Scalar() == "world_coord_sys") {
            const YAML::Node& coordsys_node = it->second;
            YAML_CoordinateSystem coordsys_yaml;
            if (!coordsys_yaml.parse(coordsys_node)) {
                return false;
            }
            coordsystems_yaml.push_back(coordsys_yaml);
        }
    }
    return true;
}

bool YAML_CoordinateSystem::parse(const YAML::Node& camera) {
    if (camera) {
        if (camera["origin"] && camera["origin"].size() == 3) {
            YAML::Node nlocation = camera["origin"];
            origin.x = nlocation[0].as<float>();
            origin.y = nlocation[1].as<float>();
            origin.z = nlocation[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: Coordinate system has no origin." << std::endl;
            return false;
        }
        if (camera["up"] && camera["up"].size() == 3) {
            YAML::Node nlookat = camera["up"];
            up.x = nlookat[0].as<float>();
            up.y = nlookat[1].as<float>();
            up.z = nlookat[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: camera has no up vector." << std::endl;
            return false;
        }
        if (camera["front"] && camera["front"].size() == 3) {
            YAML::Node nlookat = camera["front"];
            front.x = nlookat[0].as<float>();
            front.y = nlookat[1].as<float>();
            front.z = nlookat[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: camera has no front vector." << std::endl;
            return false;
        }
    }
    return true;
}

bool YAML_VisibilityVolume::parse(const YAML::Node& visibility) {
    if (visibility) {
        if (visibility["width"]) {
            width = visibility["width"].as<int>();
        } else {
            std::cout << "Error: Visibility volume has no width resolution." << std::endl;
            return false;
        }
        if (visibility["height"]) {
            height = visibility["height"].as<int>();
        } else {
            std::cout << "Error: Visibility volume has no height resolution." << std::endl;
            return false;
        }
        if (visibility["origin"] && visibility["origin"].size() == 3) {
            YAML::Node nlocation = visibility["origin"];
            origin.x = nlocation[0].as<float>();
            origin.y = nlocation[1].as<float>();
            origin.z = nlocation[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: Visibility volume has no origin - Default (0,0,0) used." << std::endl;
            //return false;
        }
        if (visibility["up"] && visibility["up"].size() == 3) {
            YAML::Node nup = visibility["up"];
            up.x = nup[0].as<float>();
            up.y = nup[1].as<float>();
            up.z = nup[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: Visibility volume has no up vector - Default (0,1,0) used." << std::endl;
            //return false;
        }
        if (visibility["front"] && visibility["front"].size() == 3) {
            YAML::Node nfront = visibility["front"];
            front.x = nfront[0].as<float>();
            front.y = nfront[1].as<float>();
            front.z = nfront[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: Visibility volume has no front vector - Default (1,0,0) used." << std::endl;
            //return false;
        }
        if (visibility["fov_degrees"]) {
            fov_degrees = visibility["fov_degrees"].as<float>();
        } else {
            std::cout << "Error: Visibility volume has no field of view - Default (" << fov_degrees << ") used." << std::endl;
            //return false;
        }
        if (visibility["up_max"]) {
            up_max = visibility["up_max"].as<float>();
        } else {
            std::cout << "Error: Visibility volume has no max up (altitude) value - Default(" << up_max << ") used." << std::endl;
            //return false;
        }
        if (visibility["up_min"]) {
            up_min = visibility["up_min"].as<float>();
        } else {
            std::cout << "Error: Visibility volume has no min up (altitude) value - Default(" << up_min << ") used." << std::endl;
            //return false;
        }
        if (visibility["radius_max"]) {
            radius_max = visibility["radius_max"].as<float>();
        } else {
            std::cout << "Error: Visibility volume has no max radius value - Default(" << radius_max << ") used." << std::endl;
            //return false;
        }
        if (visibility["output_file"]) {
            output_filename = visibility["output_file"].as<std::string>();
        } else {
            std::cout << "Error: Visibility volume missing required output filename - Default(\"" << output_filename << "\") used." << std::endl;
            //return false;
        }
    }
    return true;
}

// YAML Geometry
bool YAML_Object3D::parse(const YAML::Node& object3d) {
    if (object3d) {
        if (object3d["position"] && object3d["position"].size() == 3) {
            YAML::Node nposition = object3d["position"];
            position.x = nposition[0].as<float>();
            position.y = nposition[1].as<float>();
            position.z = nposition[2].as<float>();
            //std::cout << "location = (" << geometry_yaml.location.x << ", " << geometry_yaml.location.y << ", " << geometry_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: geometry missing position. Setting it to (0,0,0)." << std::endl;
            //return false;
        }
        if (object3d["orientation axis, angle"] && object3d["orientation axis, angle"].size() == 4) {
            YAML::Node nOrientation_axis_angle = object3d["orientation axis, angle"];
            orientation_axis_angle.x = nOrientation_axis_angle[0].as<float>();
            orientation_axis_angle.y = nOrientation_axis_angle[1].as<float>();
            orientation_axis_angle.z = nOrientation_axis_angle[2].as<float>();
            orientation_axis_angle.w = nOrientation_axis_angle[3].as<float>();
            //std::cout << "location = (" << geometry_yaml.location.x << ", " << geometry_yaml.location.y << ", " << geometry_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: geometry missing orientation axis, angle. Setting it to y-axis (up) (0,1,0) angle = 0." << std::endl;
        }
        if (object3d["scale"]) {
            YAML::Node nscalef = object3d["scale"];
            if (nscalef.IsScalar()) {
                scale.x = object3d["scale"].as<float>();
                scale.y = scale.x;
                scale.z = scale.x;
            } else {
                scale.x = nscalef[0].as<float>();
                scale.y = nscalef[1].as<float>();
                scale.z = nscalef[2].as<float>();
            }
        } else {
            std::cout << "Error: geometry missing scale. Setting it to 1.0." << std::endl;
        }
    }
    return true;
}

glm::mat4 YAML_Object3D::getTransform() {
    glm::vec3 axis = glm::normalize(glm::vec3(orientation_axis_angle.x, orientation_axis_angle.y, orientation_axis_angle.z));
    glm::mat4 transform;
    if (orientation_axis_angle.w == 0 || std::abs(glm::length(axis) - 1.0f) > 1e-3f) {
        transform = glm::mat4(1.0f);
    } else {
        glm::rotate(transform, orientation_axis_angle.w * glm::pi<float>() / 180.0f, axis);
    }
    transform[0][3] = position.x;
    transform[1][3] = position.y;
    transform[2][3] = position.z;
    float scales[] = {scale.x, scale.y, scale.z};
    if (scale.x != 1.0f || scale.y != 1.0f || scale.z != 1.0f) {
        for (int colIdx = 0; colIdx < 3; colIdx++) {
            for (int rowIdx = 0; rowIdx < 3; rowIdx++) {
                transform[colIdx + 4 * rowIdx] *= scales[colIdx];
            }
        }
    }
    return transform;
}

bool YAML_Mesh::parse(const YAML::Node & mesh) {
    if (mesh) {
        YAML_Object::parse(mesh);
        YAML_Object3D::parse(mesh);
        if (mesh["format"]) {
            format = mesh["format"].as<std::string>();
            if (format == "OBJ") {
            } else if (format == "PLY") {
                //} else if (format == "DAE") {
            } else {
                std::cout << "Error: mesh missing required format." << std::endl;
                return false;
            }
        }
        if (mesh["filename"]) {
            filename = mesh["filename"].as<std::string>();
        } else {
            std::cout << "Error: mesh missing required mesh filename." << std::endl;
            return false;
        }
    }
    return true;
}
