#pragma once
#define CGLTF_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#include "cgltf.h"
#include <vector>

struct GLTFVec3 {
    float x, y, z;
};

struct GLTFTriangle {
    uint32_t a, b, c;
    float red, green, blue;
};

struct GLTFMatrix {
    float m[16];
};

struct GLTFFrame {
    std::vector<GLTFVec3> positions;
    std::vector<GLTFTriangle> triangles;
    GLTFMatrix view, projection;
    float znear, zfar;
};

std::vector< GLTFFrame> ImportGLTF(const char* szFilename, const char* cameraName)
{
    std::vector< GLTFFrame> gltfFrames;
    cgltf_options options = {  };
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, szFilename, &data);
    if (result == cgltf_result_success)
    {
        result = cgltf_load_buffers(&options, data, szFilename);

        // get max frames
        int frameCount = 1;
        for (cgltf_size anim = 0; anim < data->animations_count; anim++)
        {
            const auto& animation = data->animations[anim];
            for (cgltf_size samp = 0; samp < animation.samplers_count; samp++)
            {
                auto& sampler = animation.samplers[samp];
                frameCount = std::max((size_t)frameCount, sampler.output->count);
            }
        }
        gltfFrames.resize(frameCount);

        for (int frame = 0; frame < frameCount; frame++)
        {
            // build animation
            for (cgltf_size anim = 0; anim < data->animations_count; anim++)
            {
                const auto& animation = data->animations[anim];

                for (cgltf_size samp = 0; samp < animation.samplers_count; samp++)
                {
                    auto& sampler = animation.samplers[samp];
                   
                    cgltf_accessor* accessor = sampler.output;
                    if (frame < accessor->count)
                    {
                        auto& channel = animation.channels[samp];
                        auto* node = channel.target_node;

                        switch (channel.target_path) {

                        case cgltf_animation_path_type_translation:
                            cgltf_accessor_read_float(accessor, frame, node->translation, 3);
                            break;
                        case cgltf_animation_path_type_rotation:
                            cgltf_accessor_read_float(accessor, frame, node->rotation, 4);
                            break;
                        case cgltf_animation_path_type_scale:
                            cgltf_accessor_read_float(accessor, frame, node->scale, 3);
                            break;
                        }
                    }
                }
            }
            // render node
            auto& gltfFrame = gltfFrames[frame];
            for (cgltf_size nodeIndex = 0; nodeIndex < data->nodes_count; nodeIndex++)
            {
                const auto& node = data->nodes[nodeIndex];
                if (node.camera) {
                    if (!strcmp(node.name, cameraName))
                    {
                        Imm::matrix* p = (Imm::matrix*)&gltfFrame.projection;
                        gltfFrame.znear = node.camera->data.perspective.znear;
                        gltfFrame.zfar = node.camera->data.perspective.zfar;
                        p->glhPerspectivef2Rad(node.camera->data.perspective.yfov * 0.5f, 320.f / 200.f, gltfFrame.znear, gltfFrame.zfar, true);
                        cgltf_node_transform_world(&node, gltfFrame.view.m);
                    }
                }
                else if (node.mesh) {
                    struct Vertex
                    {
                        float x, y, z;
                        float r, g, b, a;
                    };
                    std::vector<Vertex> vertices;
                    const auto& mesh = *node.mesh;

                    // position and color
                    for (auto attributeIndex = 0; attributeIndex < mesh.primitives->attributes_count; attributeIndex++) {
                        const auto& attribute = mesh.primitives->attributes[attributeIndex];
                        
                        cgltf_accessor* accessor = attribute.data;
                        vertices.resize(accessor->count);
                        switch (attribute.type)
                        {
                        case cgltf_attribute_type_position:
                            for (int pos = 0; pos < attribute.data->count; pos++) {
                                cgltf_accessor_read_float(accessor, pos, &vertices[pos].x, 3);
                            }
                            break;
                        case cgltf_attribute_type_color:
                            for (int col = 0; col < accessor->count; col++) {
                                cgltf_accessor_read_float(accessor, col, &vertices[col].r, 4);
                            }
                            break;
                        }
                    }

                    auto meshVertexBase = gltfFrame.positions.size();

                    // transform / copy positions
                    Imm::matrix worldMat;
                    cgltf_node_transform_world(&node, worldMat.m16);
                    for (const auto& position : vertices) {
                        Imm::vec4 p{ position.x, position.y, position.z, 0.f };
                        p.TransformPoint(worldMat);
                        gltfFrame.positions.push_back({p.x, p.y, p.z});
                    }

                    for (cgltf_size primitiveIndex = 0; primitiveIndex < mesh.primitives_count; ++primitiveIndex)
                    {
                        cgltf_primitive* primitive = &mesh.primitives[primitiveIndex];

                        cgltf_accessor* accessor = primitive->indices;

                        for (cgltf_size v = 0; v < accessor->count; v += 3)
                        {
                            int components[3];
                            for (int i = 0; i < 3; ++i)
                            {
                                components[i] = int32_t(cgltf_accessor_read_index(accessor, v + i));
                            }

                            gltfFrame.triangles.push_back({});
                            auto& tri = gltfFrame.triangles.back();
                            tri.a = components[0] + meshVertexBase;
                            tri.b = components[1] + meshVertexBase;
                            tri.c = components[2] + meshVertexBase;
                            tri.red = (vertices[components[0]].r + vertices[components[1]].r + vertices[components[2]].r) / 3.f;
                            tri.green = (vertices[components[0]].g + vertices[components[1]].g + vertices[components[2]].g) / 3.f;
                            tri.blue = (vertices[components[0]].b + vertices[components[1]].b + vertices[components[2]].b) / 3.f;
                        }
                    }
                }
            }
        }
        cgltf_free(data);
    }
    
    return gltfFrames;
}
