#pragma once

#include <memory>
#include <vector>

#include "../ECS/ComponentRegistry.h"

namespace Squid {
namespace Core {

    class SceneGraph {
    public:
        SceneGraph();
        ~SceneGraph();

        void Update() const;

        void Attach(Entity entity, Entity parent, bool child_already_in_local_space = false);
		void Detach(Entity entity);
		void DetachChildren(Entity parent);

        ComponentRegistry<TransformComponent> transforms;
        ComponentRegistry<HierarchyComponent> hierarchy;
    };

} // namespace Core
} // namespace Squid