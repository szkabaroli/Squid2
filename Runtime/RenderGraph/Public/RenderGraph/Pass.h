#pragma once
#include <pch.h>
#include <RHI/Handles.h>

#include <string>
#include <functional>

namespace Squid {
namespace RenderGraph {

    class Graph;
    class Builder;
    class ResourceBase;

    class PassBase {
        friend Graph;
        friend Builder;

    public:
        explicit PassBase(const std::string &name) : name(name){};

        PassBase(const PassBase &that) = delete;
        PassBase(PassBase &&temp) = default;
        virtual ~PassBase() = default;
        PassBase &operator=(const PassBase &that) = delete;
        PassBase &operator=(PassBase &&temp) = default;

        inline const std::string &GetName() const { return name; }

    protected:
        virtual void Setup(Builder &builder) = 0;
        virtual void Execute() const = 0;
        std::string name;
        uint32_t ref_count;

        std::vector<const ResourceBase *> reads;   // resources we're reading from
        std::vector<const ResourceBase *> writes;  // resources we're writing to
        std::vector<const ResourceBase *> creates; // resources we're sampling from
    };

    template <typename DataType>
    class Pass : public PassBase {
    public:
        explicit Pass(
            const std::string &name,
            const std::function<void(DataType &, Builder &)> &setup,
            const std::function<void(const DataType &)> &execute)
            : PassBase(name), setup_fn(setup), execute_fn(execute) {}

        Pass(const Pass &that) = delete;
        Pass(Pass &&temp) = default;
        ~Pass() = default;
        Pass &operator=(const Pass &that) = delete;
        Pass &operator=(Pass &&temp) = default;

        inline DataType GetData() const { return data; }

    protected:
        DataType data;
        void Setup(Builder &builder) override { setup_fn(data, builder); }
        void Execute() const override { execute_fn(data); }
        const std::function<void(DataType &, Builder &)> setup_fn;
        const std::function<void(const DataType &)> execute_fn;
    };

} // namespace RenderGraph
} // namespace Squid
