#pragma once

#include <noise.h>

#include "middleware/node_editor/node.h"


class BaseNoiseNode : public PreviewNode {
protected:
    BaseNoiseNode(noise::module::Module* module, const std::string& name);

protected:
    bool OnSetSourceNode(BaseNode* srcNode, BasePin* dstPin, bool checkOnly) override;
    void OnDelSourceNode(BaseNode* srcNode, BasePin* dstPin) override;
    void Update() override;
    bool CheckIsConsistency() noexcept override;
    bool DrawSettings() override;

    virtual bool OnDrawSettingsImpl() { return false; }
    virtual const noise::module::Module** GetSourceModules() noexcept = 0;

    noise::module::Module* m_module = nullptr;
};

class BillowNode : public BaseNoiseNode, private noise::module::Billow {
public:
    BillowNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class CheckerboardNode : public BaseNoiseNode, private noise::module::Checkerboard {
public:
    CheckerboardNode();
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class ConstNode : public BaseNoiseNode, private noise::module::Const {
public:
    ConstNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class CylindersNode : public BaseNoiseNode, private noise::module::Cylinders {
public:
    CylindersNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class PerlinNode : public BaseNoiseNode, private noise::module::Perlin {
public:
    PerlinNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class RidgedMultiNode : public BaseNoiseNode, private noise::module::RidgedMulti {
public:
    RidgedMultiNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class SpheresNode : public BaseNoiseNode, private noise::module::Spheres {
public:
    SpheresNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class VoronoiNode : public BaseNoiseNode, private noise::module::Voronoi {
public:
    VoronoiNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class AbsNode : public BaseNoiseNode, private noise::module::Abs {
public:
    AbsNode();
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class ClampNode : public BaseNoiseNode, private noise::module::Clamp {
public:
    ClampNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

// TODO: Curve
class ExponentNode : public BaseNoiseNode, private noise::module::Exponent {
public:
    ExponentNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class InvertNode : public BaseNoiseNode, private noise::module::Invert {
public:
    InvertNode();
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class ScaleBiasNode : public BaseNoiseNode, private noise::module::ScaleBias {
public:
    ScaleBiasNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};

class SelectNode : public BaseNoiseNode, private noise::module::Select {
public:
    SelectNode();
    bool OnDrawSettingsImpl() override;
    const noise::module::Module** GetSourceModules() noexcept override { return m_pSourceModule; }
};
