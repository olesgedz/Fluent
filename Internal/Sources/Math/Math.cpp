#include "Math/Math.hpp"

namespace Fluent
{
    float Radians(float degrees)
    {
        glm::radians(degrees);
    }

    Matrix4 CreatePerspectiveMatrix(float fov, float aspect, float zNear, float zFar)
    {
        return glm::perspective(fov, aspect, zNear, zFar);
    }

    Matrix4 CreateLookAtMatrix(const Vector3& position, const Vector3& direction, const Vector3& up)
    {
        return glm::lookAt(position, position + direction, up);
    }

    Matrix4 Rotate(const Matrix4& mat, float angle, Vector3 axis)
    {
        return glm::rotate(mat, angle, axis);
    }

    Matrix4 Translate(const Matrix4& mat, const Vector3& v)
    {
        return glm::translate(mat, v);
    }
} // namespace Fluent
