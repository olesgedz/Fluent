#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Fluent
{
    using Vector2 = glm::vec2;
    using Vector3 = glm::vec3;
    using Vector4 = glm::vec4;

    using VectorInt2 = glm::ivec2;
    using VectorInt3 = glm::ivec3;
    using VectorInt4 = glm::ivec4;

    using Matrix2 = glm::mat2;
    using Matrix3 = glm::mat3;
    using Matrix4 = glm::mat4;

    Matrix4 CreatePerspectiveMatrix(float fov, float aspect, float zNear, float zFar);
    Matrix4 CreateLookAtMatrix(const Vector3& position, const Vector3& direction, const Vector3& up);
    Matrix4 Rotate(const Matrix4& mat, float angle, Vector3 axis);
    Matrix4 Translate(const Matrix4& mat, const Vector3& v);
}