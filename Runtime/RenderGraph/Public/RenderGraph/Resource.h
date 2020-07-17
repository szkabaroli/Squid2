#pragma once
#include <pch.h>
#include <RHI/Handles.h>

namespace Squid { namespace RenderGraph {

    class Graph;
    class Builder;
    class PassBase;

    class ResourceBase {
        friend Graph;
        friend Builder;

    public:
        explicit ResourceBase(const std::string &name, const PassBase *creator)
            : name(name), creator(creator), ref_count(0) {
            static std::size_t static_id = 0;
            id = static_id++;
        }

        ResourceBase(const ResourceBase &that) = delete;
        ResourceBase(ResourceBase &&temp) = default;
        virtual ~ResourceBase() = default;
        ResourceBase &operator=(const ResourceBase &that) = delete;
        ResourceBase &operator=(ResourceBase &&temp) = default;

        inline std::size_t GetID() const { return id; }
        inline const std::string &GetName() const { return name; }
        inline bool IsTransient() const { return creator != nullptr; }

    protected:
        std::size_t id;
        std::string name;
        std::size_t ref_count;

        const PassBase *creator;
        std::vector<const PassBase *> readers;
        std::vector<const PassBase *> writers;
    };

    template <typename HandleType>
    class Resource : public ResourceBase {
    public:
        using ResourceHandle = HandleType;

        explicit Resource(
            const std::string &name, const PassBase *creator, const HandleType &handle)
            : ResourceBase(name, creator), handle(handle) {
            static_assert(
                std::is_base_of<RHI::Handle, HandleType>::value,
                "Resource HandleType must derive from Handle");
            // Transient (normal) constructor.
        }

        explicit Resource(const std::string &name, const HandleType &handle)
            : ResourceBase(name, nullptr), handle(handle) {
            static_assert(
                std::is_base_of<RHI::Handle, HandleType>::value,
                "Resource HandleType must derive from Handle");
            // Retained (import) constructor.
        }

        Resource(const Resource &that) = delete;
        Resource(Resource &&temp) = default;
        ~Resource() = default;
        Resource &operator=(const Resource &that) = delete;
        Resource &operator=(Resource &&temp) = default;

        inline HandleType &Get() { return handle; }

    private:
        HandleType handle;
        bool realized = false;
    };

}} // namespace Squid::RenderGraph
