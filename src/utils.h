#pragma once
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "engine/Entity.h"
#include "engine/EntityManager.h"
#include "engine/UIObject.h"

inline std::filesystem::path resolvePath(const std::string& relative) {
    namespace fs = std::filesystem;
    fs::path relPath(relative);
    fs::path base = fs::current_path();
    for (int i = 0; i < 6; ++i) {
        if (!base.empty()) {
            fs::path candidate = base / relPath;
            std::error_code ec;
            if (fs::exists(candidate, ec)) {
                fs::path canonicalPath = fs::canonical(candidate, ec);
                return ec ? candidate : canonicalPath;
            }
            if (!base.has_parent_path()) {
                break;
            }
            base = base.parent_path();
        }
    }
    return relPath;
}

inline std::vector<char> readFile(const std::string& filename) {
    const std::filesystem::path resolvedPath = resolvePath(filename);
    std::ifstream file(resolvedPath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + resolvedPath.string());
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

inline float convertComponent(float value, float parentReference) {
    if (std::abs(value) <= 1.0f) {
        return value * parentReference;
    }
    return value;
}

struct LayoutRect {
    glm::vec2 pos;
    glm::vec2 size;
};

inline LayoutRect resolveDesignRect(UIObject* obj, const LayoutRect& parentRect) {
    glm::vec2 sizeInput = obj->getSize();
    glm::vec2 sizeDesign(
        convertComponent(sizeInput.x, parentRect.size.x),
        convertComponent(sizeInput.y, parentRect.size.y)
    );
    sizeDesign = glm::max(sizeDesign, glm::vec2(1.0f));

    glm::vec2 offset(
        convertComponent(obj->getPosition().x, parentRect.size.x),
        convertComponent(obj->getPosition().y, parentRect.size.y)
    );

    glm::vec2 topLeft(parentRect.pos);
    const glm::ivec2 corner = obj->getCorner();
    if (corner.x == 2) {
        topLeft.x += parentRect.size.x - offset.x - sizeDesign.x;
    } else if (corner.x == 1) {
        topLeft.x += (parentRect.size.x - sizeDesign.x) * 0.5f + offset.x;
    } else {
        topLeft.x += offset.x;
    }
    if (corner.y == 2) {
        topLeft.y += parentRect.size.y - offset.y - sizeDesign.y;
    } else if (corner.y == 1) {
        topLeft.y += (parentRect.size.y - sizeDesign.y) * 0.5f + offset.y;
    } else {
        topLeft.y += offset.y;
    }

    return {topLeft, sizeDesign};
}

inline LayoutRect toPixelRect(const LayoutRect& designRect, const glm::vec2& canvasOrigin, float layoutScale) {
    LayoutRect pixelRect;
    pixelRect.pos = canvasOrigin + designRect.pos * layoutScale;
    pixelRect.size = glm::max(designRect.size * layoutScale, glm::vec2(1.0f));
    return pixelRect;
}

inline glm::mat4 computeWorldTransform(Entity* node) {
    glm::mat4 transform(1.0f);
    std::vector<Entity*> hierarchy;
    for (Entity* current = node; current != nullptr; current = current->getParent()) {
        hierarchy.push_back(current);
    }
    for (auto it = hierarchy.rbegin(); it != hierarchy.rend(); ++it) {
        Entity* current = *it;
        transform = glm::translate(transform, current->getPosition());
        glm::vec3 rot = current->getRotation();
        transform = glm::rotate(transform, glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, current->getScale());
    }
    return transform;
}

inline glm::vec3 rotatePointAroundPivot(const glm::vec3& point, const glm::vec3& pivot, const glm::vec3& rotation) {
    if (glm::length(rotation) < 0.001f) {
        return point;
    }
    glm::vec3 translatedPoint = point - pivot;
    glm::mat4 rotMat(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotMat = glm::rotate(rotMat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotMat = glm::rotate(rotMat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec4 rotatedPoint = rotMat * glm::vec4(translatedPoint, 1.0f);
    return glm::vec3(rotatedPoint) + pivot;
}