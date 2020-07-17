#include <RHI/Module.h>
#include <RenderGraph/Graph.h>

namespace Squid { namespace Renderer {

    void add_present_pass(RenderGraph::Graph &graph) {
        struct PassData {
            RenderGraph::Resource<RHI::TextureHandle> texture;
        };

        auto pass = graph.AddPass<PassData>(
            "Present",
            [&](PassData &data, RenderGraph::Builder &builder) {

            },
            [=](PassData &data, RHI::CommandList &list) {

            });

        return;
    }

}} // namespace Squid::Renderer