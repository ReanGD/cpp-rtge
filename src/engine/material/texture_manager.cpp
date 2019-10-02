#include "engine/material/texture_manager.h"

#include <fmt/format.h>
#include "engine/material/image_loader.h"

void TextureManager::AddFindPath(const std::filesystem::path& path) {
    m_basePaths.push_back(path);
}

std::shared_ptr<Texture> TextureManager::Create(const ImageHeader& header, std::string& error) noexcept {
    Texture::Result isSuccess;
    const bool generateMipLevelsIfNeed = false;
    auto texture = std::make_shared<Texture>(Image(header, 0, nullptr), generateMipLevelsIfNeed, error, isSuccess);

    return isSuccess.value ? texture : nullptr;
}

std::shared_ptr<Texture> TextureManager::Create(const Image& image, std::string& error) noexcept {
    return Create(image, true, error);
}

std::shared_ptr<Texture> TextureManager::Create(const Image& image, bool generateMipLevelsIfNeed, std::string& error) noexcept {
    Texture::Result isSuccess;
    auto texture = std::make_shared<Texture>(image, generateMipLevelsIfNeed, error, isSuccess);

    return isSuccess.value ? texture : nullptr;
}

std::shared_ptr<Texture> TextureManager::Load(const std::filesystem::path& path, std::string& error) noexcept {
    return Load(path, true, error);
}

std::shared_ptr<Texture> TextureManager::Load(const std::filesystem::path& path, bool generateMipLevelsIfNeed, std::string& error) noexcept {
    std::filesystem::path fullPath = path;
    bool found = std::filesystem::exists(fullPath);
    if (!found) {
        for (const auto& base: m_basePaths) {
            fullPath = base / path;
            if (std::filesystem::exists(fullPath)) {
                found = true;
                break;
            }
        }
    }

    if (!found) {
        error = fmt::format("the texture of the path '{}' was not found", path.c_str());
        return nullptr;
    }

    Image image;
    if(!image_loader::Load(fullPath.c_str(), image, error)) {
        return nullptr;
    }

    Texture::Result isSuccess;
    auto texture = std::make_shared<Texture>(image, generateMipLevelsIfNeed, error, isSuccess);
    image_loader::Free(image);
    if (!isSuccess.value) {
        error = fmt::format("failed to create texture from file '{}', error: {}", fullPath.c_str(), error);
    }

    return isSuccess.value ? texture : nullptr;
}