#pragma once
#include "Entity.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace Squid {
namespace Core {

    struct TransformComponent {
        enum Flags {
            EMPTY = 0,
            DIRTY = 1 << 0,
        };

        std::string name;

        uint32_t flags = DIRTY;

        glm::vec3 scale_local = glm::vec3(1.0f);
        glm::quat rotation_local = glm::quat();
        glm::vec3 translation_local = glm::vec3(0.0f);

        glm::mat4 world = glm::mat4(1.0f);

        inline bool IsDirty() const { return flags & DIRTY; }
        inline void SetDirty(bool value = true) {
            if (value)
                flags |= DIRTY;
            else
                flags &= ~DIRTY;
        }

        // TODO
        glm::vec3 GetPosition() const {
            glm::vec3 skew;
            glm::vec4 persp;
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::decompose(world, scale, rotation, translation, skew, persp);
            return translation;
        };

        glm::quat GetRotation() const {
            glm::vec3 skew;
            glm::vec4 persp;
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::decompose(world, scale, rotation, translation, skew, persp);
            return rotation;
        };

        glm::vec3 GetScale() const {
            glm::vec3 skew;
            glm::vec4 persp;
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::decompose(world, scale, rotation, translation, skew, persp);
            return scale;
        };

        glm::mat4 GetLocalMatrix() const {
            // Identity
            glm::mat4 matrix = glm::mat4(1.0f);

            matrix = glm::translate(matrix, translation_local);
            matrix *= glm::mat4_cast(rotation_local);
            matrix = glm::scale(matrix, scale_local);

            return matrix;
        }

        void UpdateTransform() {
            if (IsDirty()) {
                SetDirty(false);
                world = GetLocalMatrix();
            }
        };

        void UpdateTransformParented(const TransformComponent &parent) {
            glm::mat4 w = GetLocalMatrix();
            glm::mat4 w_parent = parent.world;
            w = w * w_parent;
            world = w;
        };

        void ApplyTransform() {
            SetDirty();

            glm::vec3 skew;
            glm::vec4 persp;
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::decompose(world, scale, rotation, translation, skew, persp);

            this->translation_local += translation;
            this->rotation_local += rotation;
            this->scale_local += scale;
        };

        void ClearTransform() {
            SetDirty();

            this->scale_local = glm::vec3(1.0f);
            this->rotation_local = glm::quat();
            this->translation_local = glm::vec3(0.0f);
        };

        void Translate(const glm::vec3 &value) {
            SetDirty();
            this->translation_local += value;
        };

        void Rotate(const glm::vec3 &euler) {
            SetDirty();
            rotation_local *= glm::quat(euler);
        };

        void Rotate(const glm::quat &quaterion) {
            SetDirty();
            rotation_local += quaterion;
        };

        void Scale(const glm::vec3 &value) {
            SetDirty();
            this->scale_local += value;
        };

        void MatrixTransform(const glm::mat4 &matrix) {
            SetDirty();

            glm::vec3 skew;
            glm::vec4 persp;
            glm::decompose(world, scale_local, rotation_local, translation_local, skew, persp);
        }
        // void MatrixTransform(const glm::mat4 &matrix);
    };

} // namespace Core
} // namespace Squid