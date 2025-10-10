#pragma once
#include "Model.h"
#include <iostream>
#include <fstream>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/glm.hpp>
#include "Renderer.h"

struct GeoSurface {
    uint32_t startIndex;
    uint32_t count;
};

void Model::loadFromFile(const std::string& path) {
    fastgltf::GltfDataBuffer data;
    data.FromPath(path);
    fastgltf::Asset gltf;
    fastgltf::Parser parser{};
    constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
        | fastgltf::Options::LoadExternalBuffers;
    auto load = parser.loadGltfBinary(data, path, gltfOptions);
    if (load) {
        gltf = std::move(load.get());
    } else {
        std::cerr<< "Failed to load glTF file: " << path << " Error: " << static_cast<std::uint64_t>(load.error()) << std::endl;
        return;
    }
    if (gltf.meshes.empty()) {
        std::cerr << "No meshes found in glTF file: " << path << std::endl;
        return;
    }
    const auto& mesh = gltf.meshes[0];
    if (mesh.primitives.empty()) {
        std::cerr << "No primitives found in mesh: " << mesh.name << std::endl;
        return;
    }
    constexpr std::size_t floatsPerVertex = 8; // position (3) + normal (3) + uv (2)
    for (const auto& primitive : mesh.primitives) {
        if (!primitive.indicesAccessor.has_value()) {
            std::cerr << "Primitive missing index accessor in glTF file: " << path << std::endl;
            continue;
        }
        const auto positionAttr = primitive.findAttribute("POSITION");
        if (positionAttr == primitive.attributes.end()) {
            std::cerr << "Primitive missing POSITION attribute in glTF file: " << path << std::endl;
            continue;
        }
        const fastgltf::Accessor& indexAccessor = gltf.accessors[primitive.indicesAccessor.value()];
        const fastgltf::Accessor& positionAccessor = gltf.accessors[positionAttr->accessorIndex];
        const std::size_t vertexCount = positionAccessor.count;
        const std::size_t initialVertexCount = vertices.size() / floatsPerVertex;
        const std::size_t startFloat = vertices.size();
        vertices.resize(vertices.size() + vertexCount * floatsPerVertex, 0.0f);
        for (std::size_t i = 0; i < vertexCount; ++i) {
            const std::size_t base = startFloat + i * floatsPerVertex;
            vertices[base + 3] = 0.0f;
            vertices[base + 4] = 0.0f;
            vertices[base + 5] = 1.0f;
            vertices[base + 6] = 0.0f;
            vertices[base + 7] = 0.0f;
        }
        indices.reserve(indices.size() + indexAccessor.count);
        fastgltf::iterateAccessor<std::uint32_t>(gltf, gltf.accessors[primitive.indicesAccessor.value()],
            [&](std::uint32_t idx) {
                indices.push_back(static_cast<uint32_t>(idx + initialVertexCount));
            });
        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, positionAccessor,
            [&](glm::vec3 v, std::size_t index) {
                const std::size_t base = startFloat + index * floatsPerVertex;
                vertices[base + 0] = v.x;
                vertices[base + 1] = v.y;
                vertices[base + 2] = v.z;
            });
        const auto normalAttr = primitive.findAttribute("NORMAL");
        if (normalAttr != primitive.attributes.end()) {
            const fastgltf::Accessor& normalAccessor = gltf.accessors[normalAttr->accessorIndex];
            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, normalAccessor,
                [&](glm::vec3 n, std::size_t index) {
                    const std::size_t base = startFloat + index * floatsPerVertex;
                    vertices[base + 3] = n.x;
                    vertices[base + 4] = n.y;
                    vertices[base + 5] = n.z;
                });
        }
        const auto uvAttr = primitive.findAttribute("TEXCOORD_0");
        if (uvAttr != primitive.attributes.end()) {
            const fastgltf::Accessor& uvAccessor = gltf.accessors[uvAttr->accessorIndex];
            fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, uvAccessor,
                [&](glm::vec2 uv, std::size_t index) {
                    const std::size_t base = startFloat + index * floatsPerVertex;
                    vertices[base + 6] = uv.x;
                    vertices[base + 7] = uv.y;
                });
        }
    }
    Renderer::Renderer* renderer = Renderer::getInstance();
    renderer->createBuffer(vertices.size() * sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    renderer->createBuffer(indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    VkBuffer stagingVertexBuffer;
    VkDeviceMemory stagingVertexBufferMemory;
    renderer->createBuffer(vertices.size() * sizeof(float), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingVertexBuffer, stagingVertexBufferMemory);
    void* data;
    vkMapMemory(renderer->getDevice(), stagingVertexBufferMemory, 0, vertices.size() * sizeof(float), 0, &data);
    memcpy(data, vertices.data(), vertices.size() * sizeof(float));
    vkUnmapMemory(renderer->getDevice(), stagingVertexBufferMemory);
    renderer->copyBuffer(stagingVertexBuffer, vertexBuffer, vertices.size() * sizeof(float));
    VkBuffer stagingIndexBuffer;
    VkDeviceMemory stagingIndexBufferMemory;
    renderer->createBuffer(indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexBufferMemory);
    vkMapMemory(renderer->getDevice(), stagingIndexBufferMemory, 0, indices.size() * sizeof(uint32_t), 0, &data);
    memcpy(data, indices.data(), indices.size() * sizeof(uint32_t));
    vkUnmapMemory(renderer->getDevice(), stagingIndexBufferMemory);
    renderer->copyBuffer(stagingIndexBuffer, indexBuffer, indices.size() * sizeof(uint32_t));
    vkDestroyBuffer(renderer->getDevice(), stagingIndexBuffer, nullptr);
    vkFreeMemory(renderer->getDevice(), stagingIndexBufferMemory, nullptr);
    vkDestroyBuffer(renderer->getDevice(), stagingVertexBuffer, nullptr);
    vkFreeMemory(renderer->getDevice(), stagingVertexBufferMemory, nullptr);
}