#include <Public/Core/ECS/Scene.h>

namespace Squid {
namespace Core {

    SceneGraph::SceneGraph() {}
    SceneGraph::~SceneGraph() {}

    void SceneGraph::Update() const {}

    void SceneGraph::Attach(Entity entity, Entity parent, bool child_already_in_local_space) {
        assert(entity != parent);

        if (hierarchy.Contains(entity)) {
            Detach(entity);
        }

        // Add a new hierarchy component to the end of container:
        hierarchy.Create(entity)->parent_id = parent;

        // Detect breaks in the tree and fix them:
        // when children are before parents, we move the parents before the children while keeping
        // ordering of other components intact
        if (hierarchy.GetCount() > 1) {
            for (auto i = hierarchy.GetCount() - 1; i > 0; --i) {

                Entity parent_candidate_entity = hierarchy.GetEntity(i);
                const HierarchyComponent &parent_candidate = hierarchy[i];

                for (auto j = 0; j < i; ++j) {
                    const HierarchyComponent &child_candidate = hierarchy[j];

                    if (child_candidate.parent_id == parent_candidate_entity) {
                        hierarchy.MoveItem(i, j);
                        ++i;
                        // next outer iteration will check the same index again as parent
                        // candidate, however things were moved upwards, so it will be a
                        // different entity!
                        break;
                    }
                }
            }
        }

        // Re-query parent after potential MoveItem(), because it invalidates references:
        HierarchyComponent &parent_component = *hierarchy.GetComponent(entity);

        TransformComponent *transform_parent = transforms.GetComponent(parent);
        if (transform_parent == nullptr) {
            transform_parent = transforms.Create(parent);
        }

        TransformComponent *transform_child = transforms.GetComponent(entity);
        if (transform_child == nullptr) {
            transform_child = transforms.Create(entity);
            transform_parent = transforms.GetComponent(parent);
            // after transforms.Create(), transform_parent
            // pointer could have become invalidated!
        }

        if (!child_already_in_local_space) {
            auto b = glm::inverse(transform_parent->world);
            transform_child->MatrixTransform(b);
            transform_child->UpdateTransform();
        }
        transform_child->UpdateTransformParented(*transform_parent);
    }

    void SceneGraph::Detach(Entity entity) {}

} // namespace Core
} // namespace Squid