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

// YAML_Scene

bool YAML_Config::parse(const std::string filenameYAMLConfig) {
    YAML::Node doc = YAML::LoadFile(filenameYAMLConfig);

    std::vector<YAML_Camera>& cameras_yaml = cameras;
    std::vector<YAML_Primitive>& primitives_yaml = primitives;
    std::vector<YAML_Mesh>& meshes_yaml = meshes;
    std::vector<YAML_Light>& lights_yaml = lights;

    for (YAML::iterator it = doc.begin(); it != doc.end(); ++it) {
        std::cout << it->first << std::endl;
        std::cout << it->second << std::endl;
        if (it->first.Scalar() == "camera") {
            const YAML::Node& camera_node = it->second;
            YAML_Camera& camera_yaml = camera;
            if (!camera_yaml.parse(camera_node)) {
                return false;
            }
            cameras_yaml.push_back(camera_yaml);
        } else if (it->first.Scalar() == "primitive") {
            const YAML::Node& primitive_node = it->second;
            YAML_Primitive primitive_yaml;
            if (!primitive_yaml.parse(primitive_node)) {
                return false;
            }
            primitives_yaml.push_back(primitive_yaml);
        } else if (it->first.Scalar() == "mesh") {
            const YAML::Node& mesh_node = it->second;
            YAML_Mesh mesh_yaml;
            if (!mesh_yaml.parse(mesh_node)) {
                return false;
            }
            meshes_yaml.push_back(mesh_yaml);
        } else if (it->first.Scalar() == "light") {
            const YAML::Node& light_node = it->second;
            YAML_Light light_yaml;
            if (!light_yaml.parse(light_node)) {
                return false;
            }
            lights_yaml.push_back(light_yaml);
        }
    }
    return true;
}

bool YAML_Camera::parse(const YAML::Node& camera) {
    if (camera) {
        if (camera["width"]) {
            width = camera["width"].as<int>();
        } else {
            std::cout << "Error: camera has no height." << std::endl;
            return false;
        }
        if (camera["height"]) {
            height = camera["height"].as<int>();
        } else {
            std::cout << "Error: camera has height." << std::endl;
            return false;
        }
        if (camera["location"] && camera["location"].size() == 3) {
            YAML::Node nlocation = camera["location"];
            location.x = nlocation[0].as<float>();
            location.y = nlocation[1].as<float>();
            location.z = nlocation[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: camera has no location." << std::endl;
            return false;
        }
        if (camera["look at"] && camera["look at"].size() == 3) {
            YAML::Node nlookat = camera["look at"];
            lookat.x = nlookat[0].as<float>();
            lookat.y = nlookat[1].as<float>();
            lookat.z = nlookat[2].as<float>();
            //std::cout << "location = (" << camera_yaml.location.x << ", " << camera_yaml.location.y << ", " << camera_yaml.location.z << ")" << std::endl;
        } else {
            std::cout << "Error: camera has no point to look at." << std::endl;
            return false;
        }
        if (camera["type"]) {
            type = camera["type"].as<std::string>();
            if (!type.empty()) {
                if (type.compare("orthographic") == 0) {
                    if (camera["scalef"]) {
                        YAML::Node nscalef = camera["scalef"];
                        ortho_scalef.x = nscalef[0].as<float>();
                        ortho_scalef.y = nscalef[1].as<float>();
                    }
                } else if (type.compare("pinhole") == 0) {
                    if (camera["fov"]) {
                        YAML::Node nfov = camera["fov"];
                        pinhole_fov.x = nfov[0].as<float>();
                        pinhole_fov.y = nfov[1].as<float>();
                    }
                } else {
                    std::cout << "Error: camera has no type." << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

//Camera::CameraPtr YAML_Camera::makeCamera() {
//    Camera::CameraPtr camera_ptr;
//    if (type.compare("orthographic") == 0) {
//        camera_ptr = std::dynamic_pointer_cast<Camera>(std::make_shared<OrthographicCamera>());
//        camera_ptr->setScaleXY(ortho_scalef.x, ortho_scalef.y);
//    } else if (type.compare("pinhole") == 0) {
//        camera_ptr = std::dynamic_pointer_cast<Camera>(std::make_shared<PinholeCamera>());
//        camera_ptr->setFovXY(pinhole_fov.x, pinhole_fov.y);
//    }
//    camera_ptr->setViewPose(location, lookat);
//    camera_ptr->setViewport(width, height);
//    return camera_ptr;
//}

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

// YAML Primitive

bool YAML_Primitive::parse(const YAML::Node& primitive) {
    if (primitive) {
        YAML_Object::parse(primitive);
        YAML_Object3D::parse(primitive);
        //Primitive myPrimitive;
        if (primitive["type"]) {
            primitive_type = primitive["type"].as<std::string>();
            if (!primitive_type.empty()) {
                if (primitive_type.compare("Plane") == 0) {
                    //myPrimitive = Plane;
                } else if (primitive_type.compare("Box") == 0) {
                } else if (primitive_type.compare("Sphere") == 0) {

                } else if (primitive_type.compare("Torus") == 0) {
                } else {
                    std::cout << "Error: primitive missing required type." << std::endl;
                    return false;
                }
            }
        }
        if (primitive["parameters"]) {
            YAML::Node nParameters = primitive["parameters"];
            if ((primitive_type == "Plane" && nParameters.size() == 3) ||
                    (primitive_type.compare("Box") == 0 && nParameters.size() == 0) ||
                    (primitive_type.compare("Sphere") == 0 && nParameters.size() == 4) ||
                    (primitive_type.compare("Torus") == 0 && nParameters.size() == 4)) {
                for (unsigned int i = 0; i < nParameters.size(); i++) {
                    parameters.push_back(nParameters[i].as<float>());
                }
            } else {
                std::cout << "Incorrect number of parameters for " << primitive_type << "parsed size = "
                        << nParameters.size() << std::endl;
            }
        }
        if (parameters.size() == 0) {
            std::cout << "Error: primitive missing required construction parameters using defaults." << std::endl;
//            if (primitive_type == "Plane") {
//                parameters.insert(parameters.end(), Plane::defaultParameters.begin(), Plane::defaultParameters.end());
//            } else if (primitive_type.compare("Box") == 0) {
//                parameters.insert(parameters.end(), Box::defaultParameters.begin(), Box::defaultParameters.end());
//            } else if (primitive_type.compare("Sphere") == 0) {
//                parameters.insert(parameters.end(), Sphere::defaultParameters.begin(), Sphere::defaultParameters.end());
//            } else if (primitive_type.compare("Torus") == 0) {
//                parameters.insert(parameters.end(), Torus::defaultParameters.begin(), Torus::defaultParameters.end());
//            }
        }
        if (primitive["material"]) {
            material = primitive["material"].as<std::string>();
        } else {
            std::cout << "Error: primitive missing required material assuming \"Default\" material." << std::endl;
        }
    }
    return true;
}

//void YAML_Primitive::getGeometry(std::vector<VertexAttributes>& attributes, std::vector<unsigned int>& indices) {
//    //optix::Geometry primitiveGeom;
//    if (primitive_type == "Plane") {
//        Plane::create(1, 1, 1, attributes, indices);
//    } else if (primitive_type == "Box") {
//        Box::create(0, 0, attributes, indices);
//    } else if (primitive_type == "Sphere") {
//        Sphere::create(180, 90, 1.0f, M_PIf, attributes, indices);
//    } else if (primitive_type == "Torus") {
//        Torus::create(180, 180, 0.75f, 0.25f, attributes, indices);
//    }
//    //return primitiveGeom;
//}

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

//void YAML_Light::generateGeometry(std::vector<VertexAttributes>& attributes, std::vector<unsigned int>& indices) {
//    if (name == "parallelogram") {
//        Parallelogram::create(optix::make_float3(0.0f, 0.0f, 0.0f), scale.x * optix::make_float3(1.0f, 0.0f, 0.0f),
//                scale.y * optix::make_float3(0.0f, 1.0f, 0.0f), optix::make_float3(0.0f, 0.0f, 1.0f), attributes, indices);
//    }
//}

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
        if (mesh["material"]) {
            material = mesh["material"].as<std::string>();
        } else {
            std::cout << "Error: mesh missing required material assuming \"Default\" material." << std::endl;
        }
    }
    return true;
}
//
//std::vector<tinyobj::shape_t> YAML_Mesh::getShapes() {
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//    std::string meshFilename = std::string(sutil::samplesDir()) + "/data/" + filename;
//    std::string err;
//    optix::Geometry geom;
//    if (LoadObj(shapes, // [output]
//            materials, // [output]
//            err, // [output]
//            meshFilename.c_str())) {
//    }
//    return shapes;
//}

//Mesh YAML_Mesh::getMesh() {
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//    std::string meshFilename = std::string(sutil::samplesDir()) + "/data/" + filename;
//
//    Mesh mesh;
//    MeshLoader meshLoader(meshFilename);
//    meshLoader.scanMesh(mesh);
//
//    mesh.positions = new float[ 3 * mesh.num_vertices ];
//    mesh.normals = mesh.has_normals ? new float[ 3 * mesh.num_vertices ] : 0;
//    mesh.texcoords = mesh.has_texcoords ? new float[ 2 * mesh.num_vertices ] : 0;
//    mesh.tri_indices = new int32_t[ 3 * mesh.num_triangles ];
//    mesh.mat_indices = new int32_t[ 1 * mesh.num_triangles ];
//
//    mesh.mat_params = new MaterialParams[ mesh.num_materials ];
//
//    meshLoader.loadMesh(mesh);
//    return mesh;
//}

bool YAML_Light::parse(const YAML::Node & light) {
    if (!light) {
        return false;
    }
    if (!YAML_Object::parse(light)) {
        return false;
    }
    if (light["name"]) {
        name = light["name"].as<std::string>();
    } else {
        std::cout << "Error: light missing name field." << std::endl;
        return false;
    }
    if (light["intensity"]) {
        intensity = light["intensity"].as<float>();
    } else {
        std::cout << "Error: light missing intensity field. Assuming intensity = " << intensity << "." << std::endl;
    }
    if (light["color"] && light["color"].size() == 3) {
        YAML::Node nlight = light["color"];
        color.x = nlight[0].as<float>();
        color.y = nlight[1].as<float>();
        color.z = nlight[2].as<float>();

    } else {
        std::cout << "Error: light missing color field assuming color = (" <<
                color.x << "," << color.y << "," << color.z << ")." << std::endl;
    }
    if (name == "environmental") {
        // environmental lights have no position/orientation infor
        std::cout << "Environmental lighting is set to ON." << std::endl;
    } else if (name == "parallelogram") {
        if (YAML_Object3D::parse(light)) {
            // parallelogram light            
        } else {
            std::cout << "Error: Light 3D pose could not be parsed." << std::endl;
            return false;
        }
    } else {
        // allow parsing of antennas 
        //        std::cout << "Error: light having name " << name << " is not supported." << std::endl;
        //        return false;
    }
    return true;
}
