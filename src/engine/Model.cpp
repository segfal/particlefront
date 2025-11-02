#include <Model.h>
#include <Renderer.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cfloat>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct GeoSurface {
    uint32_t startIndex;
    uint32_t count;
};

void Model::loadFromFile(const std::string& path) {
    const std::filesystem::path modelPath(path);
    auto dataResult = fastgltf::GltfDataBuffer::FromPath(modelPath);
    if (!dataResult) {
        std::cerr << "Failed to open glTF file: " << path << " Error: "
                  << fastgltf::getErrorName(dataResult.error()) << std::endl;
        return;
    }
    fastgltf::GltfDataBuffer data = std::move(dataResult.get());
    fastgltf::Parser parser{};
    constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
        | fastgltf::Options::LoadExternalBuffers;
    auto load = parser.loadGltfBinary(data, modelPath.parent_path(), gltfOptions);
    if (!load) {
        std::cerr << "Failed to load glTF file: " << path << " Error: "
                  << fastgltf::getErrorName(load.error()) << std::endl;
        return;
    }
    fastgltf::Asset gltf = std::move(load.get());
    if (gltf.meshes.empty()) {
        std::cerr << "No meshes found in glTF file: " << path << std::endl;
        return;
    }
    const auto& mesh = gltf.meshes[0];
    if (mesh.primitives.empty()) {
        std::cerr << "No primitives found in mesh: " << mesh.name << std::endl;
        return;
    }
    constexpr std::size_t floatsPerVertex = 11; // position (3) + normal (3) + uv (2) + tangent (3)
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
            vertices[base + 3] = 0.0f;  // normal.x
            vertices[base + 4] = 0.0f;  // normal.y
            vertices[base + 5] = 1.0f;  // normal.z
            vertices[base + 6] = 0.0f;  // uv.x
            vertices[base + 7] = 0.0f;  // uv.y
            vertices[base + 8] = 1.0f;  // tangent.x
            vertices[base + 9] = 0.0f;  // tangent.y
            vertices[base + 10] = 0.0f; // tangent.z
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
        bool hasTangents = false;
        const auto tangentAttr = primitive.findAttribute("TANGENT");
        if (tangentAttr != primitive.attributes.end()) {
            const fastgltf::Accessor& tangentAccessor = gltf.accessors[tangentAttr->accessorIndex];
            fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, tangentAccessor,
                [&](glm::vec4 t, std::size_t index) {
                    const std::size_t base = startFloat + index * floatsPerVertex;
                    vertices[base + 8] = t.x;
                    vertices[base + 9] = t.y;
                    vertices[base + 10] = t.z;
                });
            hasTangents = true;
        }
        if (!hasTangents) {
            std::vector<glm::vec3> tangents(vertexCount, glm::vec3(0.0f));
            for (std::size_t i = initialVertexCount * 3; i < indices.size(); i += 3) {
                const uint32_t i0 = indices[i] - static_cast<uint32_t>(initialVertexCount);
                const uint32_t i1 = indices[i + 1] - static_cast<uint32_t>(initialVertexCount);
                const uint32_t i2 = indices[i + 2] - static_cast<uint32_t>(initialVertexCount);
                if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) continue;
                const std::size_t b0 = startFloat + i0 * floatsPerVertex;
                const std::size_t b1 = startFloat + i1 * floatsPerVertex;
                const std::size_t b2 = startFloat + i2 * floatsPerVertex;
                const glm::vec3 v0(vertices[b0], vertices[b0 + 1], vertices[b0 + 2]);
                const glm::vec3 v1(vertices[b1], vertices[b1 + 1], vertices[b1 + 2]);
                const glm::vec3 v2(vertices[b2], vertices[b2 + 1], vertices[b2 + 2]);
                const glm::vec2 uv0(vertices[b0 + 6], vertices[b0 + 7]);
                const glm::vec2 uv1(vertices[b1 + 6], vertices[b1 + 7]);
                const glm::vec2 uv2(vertices[b2 + 6], vertices[b2 + 7]);
                const glm::vec3 edge1 = v1 - v0;
                const glm::vec3 edge2 = v2 - v0;
                const glm::vec2 deltaUV1 = uv1 - uv0;
                const glm::vec2 deltaUV2 = uv2 - uv0;
                const float det = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
                glm::vec3 tangent(1.0f, 0.0f, 0.0f);
                if (std::abs(det) > 1e-6f) {
                    const float invDet = 1.0f / det;
                    tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * invDet;
                }
                tangents[i0] += tangent;
                tangents[i1] += tangent;
                tangents[i2] += tangent;
            }
            for (std::size_t i = 0; i < vertexCount; ++i) {
                const std::size_t base = startFloat + i * floatsPerVertex;
                glm::vec3 n(vertices[base + 3], vertices[base + 4], vertices[base + 5]);
                glm::vec3 t = tangents[i];
                t = glm::normalize(t - n * glm::dot(n, t));
                if (glm::length(t) < 0.001f) {
                    t = glm::vec3(1.0f, 0.0f, 0.0f);
                }
                vertices[base + 8] = t.x;
                vertices[base + 9] = t.y;
                vertices[base + 10] = t.z;
            }
        }
    }
    boundsMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    boundsMax = glm::vec3(-FLT_MAX,-FLT_MAX,-FLT_MAX);

    for (std::size_t i = 0; i < vertices.size(); i += floatsPerVertex){
        glm::vec3 pos(vertices[i], vertices[i + 1], vertices[i + 2]);
        boundsMin = glm::min(boundsMin, pos);
        boundsMax = glm::max(boundsMax, pos);
    }

    if (vertices.empty() || indices.empty()) {
        std::cerr << "Model " << name << " contains no vertex/index data after loading " << path << std::endl;
        return;
    }
    renderer = Renderer::getInstance();
    renderer->createBuffer(
        vertices.size() * sizeof(float),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory
    );
    renderer->createBuffer(
        indices.size() * sizeof(uint32_t),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexBufferMemory
    );
    VkBuffer stagingVertexBuffer;
    VkDeviceMemory stagingVertexBufferMemory;
    VkDeviceSize vertexBufferSize = vertices.size() * sizeof(float);
    renderer->createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingVertexBuffer, stagingVertexBufferMemory);
    void* stagingData = nullptr;
    vkMapMemory(renderer->getDevice(), stagingVertexBufferMemory, 0, vertexBufferSize, 0, &stagingData);
    memcpy(stagingData, vertices.data(), vertexBufferSize);
    vkUnmapMemory(renderer->getDevice(), stagingVertexBufferMemory);
    renderer->copyBuffer(stagingVertexBuffer, vertexBuffer, vertexBufferSize);
    VkBuffer stagingIndexBuffer;
    VkDeviceMemory stagingIndexBufferMemory;
    VkDeviceSize indexBufferSize = indices.size() * sizeof(uint32_t);
    renderer->createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexBufferMemory);
    stagingData = nullptr;
    vkMapMemory(renderer->getDevice(), stagingIndexBufferMemory, 0, indexBufferSize, 0, &stagingData);
    memcpy(stagingData, indices.data(), indexBufferSize);
    vkUnmapMemory(renderer->getDevice(), stagingIndexBufferMemory);
    renderer->copyBuffer(stagingIndexBuffer, indexBuffer, indexBufferSize);
    vkDestroyBuffer(renderer->getDevice(), stagingIndexBuffer, nullptr);
    vkFreeMemory(renderer->getDevice(), stagingIndexBufferMemory, nullptr);
    vkDestroyBuffer(renderer->getDevice(), stagingVertexBuffer, nullptr);
    vkFreeMemory(renderer->getDevice(), stagingVertexBufferMemory, nullptr);
}