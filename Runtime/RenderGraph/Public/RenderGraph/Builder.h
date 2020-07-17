#pragma once
#include <pch.h>
#include <string>

namespace Squid { namespace RenderGraph {

    class PassBase;
    class Graph;

    class Builder {
        friend class Graph;

    public:
        template <typename ResourceType, typename HandleType>
        ResourceType *Create(const std::string &name, const HandleType &handle);

        template <typename ResourceType>
        ResourceType *Read(ResourceType *resource);

        template <typename ResourceType>
        ResourceType *Write(ResourceType *resource);

    private:
        Builder(Graph *graph, PassBase *pass) : graph(graph), pass(pass){};

        Graph *graph;
        PassBase *pass;
    };

}} // namespace Squid::RenderGraph
