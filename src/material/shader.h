#pragma once

#include <string>
#include <glm/mat4x4.hpp>


class Shader {
private:
    Shader() = default;
    Shader(uint handle);
public:
    ~Shader() = default;

    void Bind() const;
    void Unbind() const;
    void Delete();

    void SetBool(const char* name, bool value) const;
    void SetInt(const char* name, int value) const;
    void SetFloat(const char* name, float value) const;
    void SetVec2(const char* name, const glm::vec2& vec) const;
    void SetVec2(const char* name, float x, float y) const;
    void SetVec3(const char* name, const glm::vec3& vec) const;
    void SetVec3(const char* name, float x, float y, float z) const;
    void SetVec4(const char* name, const glm::vec4& vec) const;
    void SetVec4(const char* name, float x, float y, float z, float w) const;
    void SetMat2(const char* name, const glm::mat2& mat) const;
    void SetMat3(const char* name, const glm::mat3& mat) const;
    void SetMat4(const char* name, const glm::mat4& mat) const;

    static std::pair<Shader, const std::string> Create(const std::string& vertexShaderName, const std::string& fragmentShaderName);
private:
    uint m_handle = 0;
};
