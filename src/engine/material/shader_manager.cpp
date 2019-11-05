#include "engine/material/shader_manager.h"

#include <map>
#include <fstream>
#include <ucl++.h>
#include <filesystem>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>
#include "engine/api/gl.h"


enum class ShaderType : uint8_t {
    Vertex = 0,
    Geometry,
    Fragment,
    Unknown
};

enum class UniformType : uint8_t {
    Vec2 = 0,
    Vec3,
    Vec4,
    Unknown
};

static UniformType FromString(const std::string& value) {
    if (value == "vec2") {
        return UniformType::Vec2;
    } else if (value == "vec3") {
        return UniformType::Vec3;
    } else if (value == "vec4") {
        return UniformType::Vec4;
    } else {
        return UniformType::Unknown;
    }
}

struct UniformInfo {
    UniformType type = UniformType::Unknown;
    std::string name;
};

class MaterialParser {
public:
    MaterialParser() = default;
    ~MaterialParser() = default;

    bool Parse(const std::filesystem::path& path, std::string& error) {
        m_shaderType = ShaderType::Unknown;
        m_funcName.clear();
        m_funcData.clear();
        m_uniforms.clear();

        if (!std::filesystem::exists(path)) {
            error = fmt::format("the material of the path '{}' was not found", path.c_str());
            return false;
        }

        std::string data;
        if (!ReadFile(path.c_str(), data, error)) {
            error = fmt::format("the material of the path '{}' was not read: {}", path.c_str(), error);
            return false;
        }

        auto root = ucl::Ucl::parse(data, error, UCL_DUPLICATE_ERROR);
        if (!root) {
            error = fmt::format("the material of the path '{}' was not parse: {}", path.c_str(), error);
            return false;
        }

        if (!ParseRoot(root, error)) {
            error = fmt::format("the material of the path '{}' was not parse: {}", path.c_str(), error);
            return false;
        }

        return true;
    }
private:
    bool ReadFile(const char* filepath, std::string& data, std::string& error) {
        std::ifstream ifs(filepath, std::ifstream::in);
        if(!ifs) {
            error = fmt::format("couldn't open file '{}', error: {}", filepath, strerror(errno));
            return false;
        }

        ifs.seekg(0, std::ios::end);
        data.reserve(static_cast<size_t>(ifs.tellg()));
        ifs.seekg(0, std::ios::beg);

        data.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        return true;
    }

    bool ParseRoot(const ucl::Ucl& section, std::string& error) {
        for (const auto &it :section) {
            if (it.key() == "material") {
                if (!ParseMaterial(it, error)) {
                    return false;
                }
            } else if (it.key() == "vertex") {
                m_shaderType = ShaderType::Vertex;
                m_funcData = it.string_value();
            } else if (it.key() == "geometry") {
                m_shaderType = ShaderType::Geometry;
                m_funcData = it.string_value();
            } else if (it.key() == "fragment") {
                m_shaderType = ShaderType::Fragment;
                m_funcData = it.string_value();
            } else {
                error = fmt::format("unknown section: {} with data: {}", it.key(), it.dump());
                return false;
            }
        }

        if (m_funcName.empty()) {
            error = "not found function name";
            return false;
        }
        if (m_funcData.empty()) {
            error = "not found function data";
            return false;
        }

        return true;
    }

    bool ParseMaterial(const ucl::Ucl& section, std::string& error) {
        for (const auto& it: section) {
            if (it.key() == "func") {
                m_funcName = it.string_value();
            } else if (it.key() == "uniforms") {
                if (!ParseUniforms(it, error)) {
                    return false;
                }
            } else {
                error = fmt::format("unknown section: {} with data: {}", it.key(), it.dump());
                return false;
            }
        }

        return true;
    }

    bool ParseUniforms(const ucl::Ucl& section, std::string& error) {
        for (const auto& uniformIt: section) {
            UniformInfo uniform;
            std::string desc = uniformIt.key();
            if (desc.empty()) {
                error = "empty uniform description";
                return false;
            }
            for (const auto& it: uniformIt) {
                if (it.key() == "type") {
                    uniform.type = FromString(it.string_value());
                    if (uniform.type == UniformType::Unknown) {
                        error = fmt::format("unknown uniform type: {} in uniform:{}", it.string_value(), desc);
                        return false;
                    }
                } else if (it.key() == "name") {
                    uniform.name = it.string_value();
                } else {
                    error = fmt::format("unknown section in uniform:{}: key={}, value={}", desc, it.key(), it.dump());
                    return false;
                }
            }

            if (uniform.name.empty()) {
                error = fmt::format("not found name for uniform:{}", desc);
                return false;
            }
            if (uniform.type == UniformType::Unknown) {
                error = fmt::format("not found type for uniform:{}", desc);
                return false;
            }
            m_uniforms[desc] = uniform;
        }

        return true;
    }

private:
    ShaderType m_shaderType = ShaderType::Unknown;
    std::string m_funcName;
    std::string m_funcData;
    std::map<std::string, UniformInfo> m_uniforms;
};

bool ShaderManager::Init(std::string& error) {
    const auto base = std::filesystem::current_path() / "materials";

    MaterialParser parser1;
    if (!parser1.Parse(base / "frag_base_clr_mat.mat", error)) {
        return false;
    }

    MaterialParser parser2;
    if (!parser2.Parse(base / "frag_light_none.mat", error)) {
        return false;
    }

    return true;
}