#include <RHI/Module.h>
#include <RenderGraph/Graph.h>

namespace Squid { namespace Renderer {

    using TextureResource = RenderGraph::Resource<RHI::TextureHandle>;



    void add_gbuffer_pass(RenderGraph::Graph &graph) {
        struct PassData {
            TextureResource g_color;
            TextureResource g_normal;
        };

        auto pass = graph.AddPass<PassData>(
            "gBuffer pass",
            [&](PassData &data, RenderGraph::Builder &builder) {
                //  data.g_color = builder.Create<TextureResource>("gColor", g_color);
                //  data.g_normal = builder.Create<TextureResource>("gNormal", g_normal);
            },
            [=](PassData &data, RHI::CommandList &list) { // list.Draw();
            });

        return;
    }

}} // namespace Squid::Renderer
