#pragma once
#include <pch.h>
#include <RHI/Module.h>
#include "Builder.h"
#include "Pass.h"
#include "Resource.h"

#include <fstream>
#include <algorithm>

namespace Squid {
    namespace RenderGraph {

        class Graph {
            friend Builder;

        public:
            virtual ~Graph() = default;

            // TODO proper argument types
            template <typename DataType, typename... ArgumentTypes>
            Pass<DataType> *AddPass(ArgumentTypes &&... arguments) {
                graph_passes.emplace_back(std::make_unique<Pass<DataType>>(arguments...));
                auto pass = graph_passes.back().get();

                Builder builder(this, pass);
                pass->Setup(builder);

                return static_cast<Pass<DataType> *>(pass);
            }

            template <typename HandleType>
            Resource<HandleType> *
            ImportResource(const std::string &name, const HandleType &handle) {
                graph_resources.emplace_back(std::make_unique<Resource<HandleType>>(name, handle));
                return static_cast<Resource<HandleType> *>(graph_resources.back().get());
            }

            void Compile() {
                // reference counters
                for (auto &pass : graph_passes)
                    pass->ref_count = static_cast<uint32_t>(pass->creates.size()) +
                                      static_cast<uint32_t>(pass->writes.size());
                for (auto &resource : graph_resources)
                    resource->ref_count = static_cast<uint32_t>(resource->readers.size());

                // TODO render pass culling

                // Timeline
                for (auto &pass : graph_passes) {
                    if (pass->ref_count == 0 /*&& !pass->cull_immune()*/)
                        continue;

                    std::vector<ResourceBase *> realized_resources, derealized_resources;

                    for (auto resource : pass->creates) {
                        realized_resources.push_back(const_cast<ResourceBase *>(resource));
                        if (resource->readers.empty() && resource->writers.empty())
                            derealized_resources.push_back(const_cast<ResourceBase *>(resource));
                    }

                    auto reads_writes = pass->reads;
                    reads_writes.insert(
                        reads_writes.end(), pass->writes.begin(), pass->writes.end());
                    for (auto resource : reads_writes) {
                        if (!resource->IsTransient())
                            continue;

                        auto valid = false;
                        std::size_t last_index;
                        if (!resource->readers.empty()) {
                            auto last_reader = std::find_if(
                                graph_passes.begin(), graph_passes.end(),
                                [&resource](const std::unique_ptr<PassBase> &iteratee) {
                                    return iteratee.get() == resource->readers.back();
                                });
                            if (last_reader != graph_passes.end()) {
                                valid = true;
                                last_index = std::distance(graph_passes.begin(), last_reader);
                            }
                        }
                        if (!resource->writers.empty()) {
                            auto last_writer = std::find_if(
                                graph_passes.begin(), graph_passes.end(),
                                [&resource](const std::unique_ptr<PassBase> &iteratee) {
                                    return iteratee.get() == resource->writers.back();
                                });
                            if (last_writer != graph_passes.end()) {
                                valid = true;
                                last_index = std::max(
                                    last_index,
                                    std::size_t(std::distance(graph_passes.begin(), last_writer)));
                            }
                        }

                        if (valid && graph_passes[last_index] == pass)
                            derealized_resources.push_back(const_cast<ResourceBase *>(resource));

                        render_steps.push_back(
                            RenderStep{pass.get(), realized_resources, derealized_resources});
                    }
                }
            };

            void Execute() {
                for (auto &step : render_steps) {
                    for (auto resource : step.realized_resources) {
                        // resource->realize();
                    }
                    step.render_pass->Execute();
                    for (auto resource : step.derealized_resources) {
                        // resource->derealize();
                    }
                }
            };

            void ExportGraphViz(const std::string &filepath) {

                std::ofstream stream(filepath);
                stream << "digraph framegraph \n{\n";

                stream << "rankdir = LR\n";
                stream << "bgcolor = black\n\n";
                stream << "node [shape=rectangle, fontname=\"helvetica\", fontsize=12]\n\n";

                for (auto &pass : graph_passes)
                    stream << "\"" << pass->GetName() << "\" [label=\"" << pass->GetName()
                           << "\\nRefs: " << pass->ref_count
                           << "\", style=filled, fillcolor=darkorange]\n";
                stream << "\n";

                for (auto &resource : graph_resources)
                    stream << "\"" << resource->GetName() << "\" [label=\"" << resource->GetName()
                           << "\\nRefs: " << resource->ref_count << "\\nID: " << resource->GetID()
                           << "\", style=filled, fillcolor= "
                           << "skyblue"
                           << "]\n";
                stream << "\n";

                for (auto &pass : graph_passes) {
                    stream << "\"" << pass->GetName() << "\" -> { ";
                    for (auto &resource : pass->creates)
                        stream << "\"" << resource->GetName() << "\" ";
                    stream << "} [color=seagreen]\n";

                    stream << "\"" << pass->GetName() << "\" -> { ";
                    for (auto &resource : pass->writes)
                        stream << "\"" << resource->GetName() << "\" ";
                    stream << "} [color=gold]\n";
                }
                stream << "\n";

                for (auto &resource : graph_resources) {
                    stream << "\"" << resource->GetName() << "\" -> { ";
                    for (auto &pass : resource->readers)
                        stream << "\"" << pass->GetName() << "\" ";
                    stream << "} [color=firebrick]\n";
                }
                stream << "}";
            }

        private:
            struct RenderStep {
                PassBase *render_pass;
                std::vector<ResourceBase *> realized_resources;
                std::vector<ResourceBase *> derealized_resources;
            };

            std::unique_ptr<RHI::Device> device;

            std::vector<std::unique_ptr<PassBase>> graph_passes; // list of frame graph passes
            std::vector<std::unique_ptr<ResourceBase>> graph_resources;

            std::vector<RenderStep> render_steps;
        };

        template <typename ResourceType, typename HandleType>
        ResourceType *Builder::Create(const std::string &name, const HandleType &handle) {
            graph->graph_resources.emplace_back(std::make_unique<ResourceType>(name, pass, handle));
            const auto resource = graph->graph_resources.back().get();
            pass->creates.push_back(resource);
            return static_cast<ResourceType *>(resource);
        }

        template <typename ResourceType>
        ResourceType *Builder::Read(ResourceType *resource) {
            resource->readers.push_back(pass);
            pass->reads.push_back(resource);
            return resource;
        }

        template <typename ResourceType>
        ResourceType *Builder::Write(ResourceType *resource) {
            resource->writers.push_back(pass);
            pass->writes.push_back(resource);
            return resource;
        }

    } // namespace RenderGraph
} // namespace Squid
