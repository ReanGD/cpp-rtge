#pragma once

#include "engine/material/image.h"
#include "middleware/node_editor/node.h"


class Texture;
class PerlinNode : public BaseNode {
public:
    enum class Quality : uint8_t {
        Fast = 0,
        Std,
        Best,
        Count,
    };
public:
    PerlinNode();
    ~PerlinNode() = default;

    bool Create(std::string& error) noexcept;
    bool DrawSettings() override;
    void DrawPreview() override;
    Image GenerateImage() const noexcept;

protected:
    double m_frequency = 1.0;              // DEFAULT_PERLIN_FREQUENCY
    double m_lacunarity = 2.0;             // DEFAULT_PERLIN_LACUNARITY
    Quality m_noiseQuality = Quality::Std; // DEFAULT_PERLIN_QUALITY
    uint8_t m_octaveCount = 6;             // DEFAULT_PERLIN_OCTAVE_COUNT
    double m_persistence = 0.5;            // DEFAULT_PERLIN_PERSISTENCE
    int32_t m_seed = 0;                    // DEFAULT_PERLIN_SEED

    std::shared_ptr<Texture> m_preview;
};