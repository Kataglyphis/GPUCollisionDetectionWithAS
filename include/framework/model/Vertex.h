#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/gtx/hash.hpp>

class Vertex {
public:
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uvCoord;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::uint matIdx;

    // Only use for debugging
    Vertex() :pos(glm::vec3(0, 0, 0)), color(glm::vec3(0, 0, 0)), uvCoord(glm::vec3(0, 0, 0)), normal(glm::vec3(0, 0, 0)),
        matIdx(0), tangent(glm::vec3(0, 0, 0)), bitangent(glm::vec3(0, 0, 0)) {}

    Vertex(glm::vec3 pos) :pos(pos),
        color(glm::vec3(0,0,0)), uvCoord(glm::vec3(0, 0, 0)), normal(glm::vec3(0, 0, 0)),
        matIdx(0), tangent(glm::vec3(0, 0, 0)), bitangent(glm::vec3(0, 0, 0)) {};



    Vertex(
        glm::vec3 pos,
        glm::vec3 color,
        glm::vec2 uvCoord,
        glm::vec3 normal,
        glm::uint matIdx,
        glm::vec3 tangent,
        glm::vec3 bitangent)
        :pos(pos), color(color), uvCoord(uvCoord), normal(normal), matIdx(matIdx), tangent(tangent), bitangent(bitangent)
    {}

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos
            && color == other.color
            && uvCoord == other.uvCoord
            && normal == other.normal
            && matIdx == other.matIdx;
    }

    void setTangentAndBitangent(glm::vec3 tangent, glm::vec3 bitangent)
    {
        this->tangent = tangent;
        this->bitangent = bitangent;
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription vertexInputBindingDescription = {};
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = sizeof(Vertex);
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return vertexInputBindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
        std::vector<VkFormat> formats = {
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32_UINT
        };
        std::vector<uint32_t> offsets = {
            offsetof(Vertex, pos),
            offsetof(Vertex, color),
            offsetof(Vertex, uvCoord),
            offsetof(Vertex, normal),
            offsetof(Vertex, tangent),
            offsetof(Vertex, bitangent),
            offsetof(Vertex, matIdx)
        };
        for (size_t i = 0; i < formats.size(); i++)
        {
            VkVertexInputAttributeDescription attributeDescription = {};
            attributeDescription.location = i;
            attributeDescription.format = formats[i];
            attributeDescription.offset = offsets[i];
            vertexInputAttributeDescriptions.push_back(attributeDescription);
        }

        return vertexInputAttributeDescriptions;
    }

};

namespace std
{
    template<> struct hash<Vertex>
    {
        size_t operator()(Vertex const& vert) const
        {
            size_t h1 = hash<glm::vec3>()(vert.pos);
            size_t h2 = hash<glm::vec3>()(vert.color);
            size_t h3 = hash<glm::vec2>()(vert.uvCoord);
            size_t h4 = hash<glm::vec3>()(vert.normal);
            size_t h5 = hash<glm::uint>()(vert.matIdx);

            return ((((((((h2 << 1) ^ h1) >> 1) ^ h3) << 1) ^ h4) >> 1) ^ h5);
        }
    };
}