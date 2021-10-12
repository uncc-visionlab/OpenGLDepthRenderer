/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <assimp/Exporter.hpp>

#include <sstream>
#include <iostream>


void exporter() {
    // create vertices and faces, then pack into an aiMesh

    aiVector3D *vertices = new aiVector3D [3]{// deleted: mesh.h:758
        { -1, -1, 0},
        { 0, 1, 0},
        { 1, -1, 0}
    };

    aiFace *faces = new aiFace[1]; // deleted: mesh.h:784
    faces[0].mNumIndices = 3;
    faces[0].mIndices = new unsigned int[3] {
        0, 1, 2
    }; // deleted: mesh.h:149

    aiMesh *mesh = new aiMesh(); // deleted: Version.cpp:150
    mesh->mNumVertices = 3;
    mesh->mVertices = vertices;
    mesh->mNumFaces = 1;
    mesh->mFaces = faces;
    mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE; // workaround, issue #3778

    // a valid material is needed, even if its empty

    aiMaterial *material = new aiMaterial(); // deleted: Version.cpp:155

    // a root node with the mesh list is needed; if you have multiple meshes, this must match.

    aiNode *root = new aiNode(); // deleted: Version.cpp:143
    root->mNumMeshes = 1;
    root->mMeshes = new unsigned int[1] {
        0
    }; // deleted: scene.cpp:77

    // pack mesh(es), material, and root node into a new minimal aiScene

    aiScene *out = new aiScene(); // deleted: by us after use
    out->mNumMeshes = 1;
    out->mMeshes = new aiMesh*[1] {
        mesh
    }; // deleted: Version.cpp:151
    out->mNumMaterials = 1;
    out->mMaterials = new aiMaterial * [1] {
        material
    }; // deleted: Version.cpp:158
    out->mRootNode = root;
    //out->mMetaData = new aiMetadata(); // workaround, issue #3781

    // and we're good to go. do whatever:

    Assimp::Exporter exporter;
    if (exporter.Export(out, "objnomtl", "triangle.obj") != AI_SUCCESS)
        std::cerr << exporter.GetErrorString() << std::endl;

    // deleting the scene will also take care of the vertices, faces, meshes, materials, nodes, etc.

    delete out;
}
