#include <Public/Renderer/Mesh.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace std {
template <>
struct hash<Squid::Renderer::MeshVertex> {
    size_t operator()(Squid::Renderer::MeshVertex const &vertex) const {
        return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.uv0) << 1);
    }
};
} // namespace std

namespace Squid {
namespace Renderer {

    Mesh::Mesh(const std::string &path) {

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

        std::unordered_map<MeshVertex, uint32_t> unique_vertices = {};

        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                MeshVertex vertex = {};
                vertex.position = {attrib.vertices[3 * index.vertex_index + 0],
                                   attrib.vertices[3 * index.vertex_index + 1],
                                   attrib.vertices[3 * index.vertex_index + 2]};

                vertex.uv0 = {attrib.texcoords[2 * index.texcoord_index + 0],
                              1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

                vertex.color = {1.0f, 1.0f, 1.0f};

                vertex.normal = {attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
                                 attrib.normals[3 * index.normal_index + 2]};

                vertices.push_back(vertex);

                if (unique_vertices.count(vertex) == 0) {
                    unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(unique_vertices[vertex]);
            }
        }

        for(const auto& vertex : vertices) {
            
        }
    }

    static void ComputeTangentBasis(
        // inputs
        std::vector<glm::vec3> &vertices,
        std::vector<glm::vec2> &uvs,
        std::vector<glm::vec3> &normals,
        // outputs
        std::vector<glm::vec3> &tangents,
        std::vector<glm::vec3> &bitangents) {

        for (int i = 0; i < vertices.size(); i += 3) {

            // Shortcuts for vertices
            glm::vec3 &v0 = vertices[i + 0];
            glm::vec3 &v1 = vertices[i + 1];
            glm::vec3 &v2 = vertices[i + 2];

            // Shortcuts for UVs
            glm::vec2 &uv0 = uvs[i + 0];
            glm::vec2 &uv1 = uvs[i + 1];
            glm::vec2 &uv2 = uvs[i + 2];

            // Edges of the triangle : position delta
            glm::vec3 deltaPos1 = v1 - v0;
            glm::vec3 deltaPos2 = v2 - v0;

            // UV delta
            glm::vec2 deltaUV1 = uv1 - uv0;
            glm::vec2 deltaUV2 = uv2 - uv0;

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
            glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
            glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

            tangents.push_back(tangent);
            tangents.push_back(tangent);
            tangents.push_back(tangent);

            // Same thing for bitangents
            bitangents.push_back(bitangent);
            bitangents.push_back(bitangent);
            bitangents.push_back(bitangent);
        }
    }

    // TODO: Avoid staging buffer on unified memory devices
    void Mesh::LoadOnDevice(const std::unique_ptr<RHI::Device> &device) {
        using RHI::BufferHandle;

        // TODO: check if already loaded

        // == VERTEX BUFFER ============================
        RHI::BufferHandle vertex_staging;
        vertex_staging.cpu_access = true;
        vertex_staging.size = sizeof(vertices[0]) * vertices.size();
        vertex_staging.usage = BufferHandle::Usage::TRANSFER_SRC;
        device->LoadBuffer(vertex_staging);

        void *vertex_data = device->MapBuffer(vertex_staging);
        memcpy(vertex_data, vertices.data(), vertex_staging.size);
        device->UnmapBuffer(vertex_staging);

        // Local vertex buffer handle
        this->vertex_buffer.cpu_access = false;
        this->vertex_buffer.size = vertex_staging.size;
        this->vertex_buffer.usage =
            (BufferHandle::Usage)(BufferHandle::Usage::TRANSFER_DST | BufferHandle::Usage::VERTEX_BUFFER);
        device->LoadBuffer(this->vertex_buffer);

        // == INDEX BUFFER ============================
        RHI::BufferHandle index_staging;
        index_staging.cpu_access = true;
        index_staging.size = sizeof(indices[0]) * indices.size();
        index_staging.usage = BufferHandle::Usage::TRANSFER_SRC;
        device->LoadBuffer(index_staging);

        void *index_data = device->MapBuffer(index_staging);
        memcpy(index_data, indices.data(), index_staging.size);
        device->UnmapBuffer(index_staging);

        // Local index buffer handle
        this->index_buffer.cpu_access = false;
        this->index_buffer.size = index_staging.size;
        this->index_buffer.usage =
            (BufferHandle::Usage)(BufferHandle::Usage::TRANSFER_DST | BufferHandle::Usage::INDEX_BUFFER);
        device->LoadBuffer(this->index_buffer);

        // == TRANSFER WORK ===========================
        RHI::CommandList work = device->BeginTransferList();
        device->Copy(work, vertex_staging, this->vertex_buffer);
        device->Copy(work, index_staging, this->index_buffer);
        // device->PrerecordList(work);
        // TODO: Transfer Queue (RHI implementation change)
        device->QueueSubmit(RHI::QueueType::GRAPHICS, work);
    }

    Mesh::~Mesh() {
        vertices.clear();
        vertices.shrink_to_fit();
        indices.clear();
        indices.shrink_to_fit();
    }

} // namespace Renderer
} // namespace Squid