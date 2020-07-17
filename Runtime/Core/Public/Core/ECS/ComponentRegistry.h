#pragma once

#include <unordered_map>
#include <vector>
#include "Entity.h"

#include "TransformComponent.h"
#include "HierarchyComponent.h"

namespace Squid {
namespace Core {

    template <typename ComponentType>
    class ComponentRegistry {
    public:
        ComponentRegistry() = default;
        ~ComponentRegistry() = default;

        inline bool Contains(Entity entity) const { return lookup.find(entity) != lookup.end(); }
        inline size_t GetCount() const { return components.size(); }
        inline Entity GetEntity(size_t index) const { return entities[index]; }

        ComponentType &operator[](size_t index) { return components[index]; }

        ComponentType *Create(Entity entity) {
            // INVALID_ENTITY is not allowed!
            assert(entity != INVALID_ENTITY);

            // Only one of this component type per entity is allowed!
            assert(lookup.find(entity) == lookup.end());

            // Entity count must always be the same as the number of components!
            assert(entities.size() == components.size());
            assert(lookup.size() == components.size());

            // Update the entity lookup table:
            lookup[entity] = components.size();

            // New components are always pushed to the end:
            components.push_back(ComponentType());

            // Also push corresponding entity:
            entities.push_back(entity);

            return &components.back();
        }

        ComponentType *GetComponent(Entity entity) {
            auto it = lookup.find(entity);
            if (it != lookup.end()) {
                return &components[it->second];
            }
            return nullptr;
        }

        void MoveItem(size_t index_from, size_t index_to) {
            assert(index_from < GetCount());
            assert(index_to < GetCount());
            if (index_from == index_to) {
                return;
            }

            // Save the moved component and entity:
            ComponentType component = std::move(components[index_from]);
            Entity entity = entities[index_from];

            // Every other entity-component that's in the way gets moved by one and lut is kept
            // updated:
            const int direction = index_from < index_to ? 1 : -1;
            for (size_t i = index_from; i != index_to; i += direction) {
                const size_t next = i + direction;
                components[i] = std::move(components[next]);
                entities[i] = entities[next];
                lookup[entities[i]] = i;
            }

            // Saved entity-component moved to the required position:
            components[index_to] = std::move(component);
            entities[index_to] = entity;
            lookup[entity] = index_to;
        }

        void Remove(Entity entity) {
            auto it = lookup.find(entity);
            if (it != lookup.end()) {
                // Directly index into components and entities array:
                const size_t index = it->second;
                const Entity entity = entities[index];

                if (index < components.size() - 1) {
                    // Swap out the dead element with the last one:
                    components[index] = std::move(components.back()); // try to use move
                    entities[index] = entities.back();

                    // Update the lookup table:
                    lookup[entities[index]] = index;
                }

                // Shrink the container:
                components.pop_back();
                entities.pop_back();
                lookup.erase(entity);
            }
        }

    private:
        std::vector<ComponentType> components;
        std::vector<Entity> entities;
        std::unordered_map<Entity, size_t> lookup;
    };

} // namespace Core
} // namespace Squid
