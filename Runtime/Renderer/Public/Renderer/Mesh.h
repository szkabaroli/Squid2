#pragma once
#include <RHI/Module.h>
#include <tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <stdio.h>
#include <string.h>

#define USE_STAGING_BUFFER 1

namespace Squid {
namespace Renderer {

    struct MeshVertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv0;

        bool operator==(const MeshVertex &other) const {
            return position == other.position && color == other.color && uv0 == other.uv0;
        }
    };

    class Mesh {
    public:
        Mesh(const std::string &path);
        ~Mesh();

        void LoadOnDevice(const std::unique_ptr<RHI::Device> &device);

        inline uint32_t GetMemoryUsage() const {
            uint32_t size = 0;
            size += uint32_t(vertices.size() * sizeof(MeshVertex));
            size += uint32_t(indices.size() * sizeof(uint16_t));
            return size;
        };

        inline uint32_t GetVerticesCount() const { return static_cast<uint32_t>(indices.size()); }
        inline const RHI::BufferHandle &GetVertexBuffer() const { return vertex_buffer; }
        inline const RHI::BufferHandle &GetIndexBuffer() const { return index_buffer; }

    private:
        uint32_t vertex_count;
        uint32_t indicies_count;

        std::vector<MeshVertex> vertices;
        std::vector<uint32_t> indices;

        // GPU Local Buffers
        RHI::BufferHandle vertex_buffer;
        RHI::BufferHandle index_buffer;
    };

} // namespace Renderer
} // namespace Squid