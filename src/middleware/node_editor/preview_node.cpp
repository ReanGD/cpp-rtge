#include "middleware/node_editor/preview_node.h"

#include <imgui.h>

#include "engine/gui/widgets.h"
#include "engine/common/exception.h"
#include "engine/material/texture_manager.h"
#include "middleware/node_editor/noiseutils.h"


PreviewNode::PreviewNode(const std::string& name)
    : BaseNode(name) {

}

PreviewNode::~PreviewNode() {
    if (m_renderedPreview != nullptr) {
        delete m_renderedPreview;
        m_renderedPreview = nullptr;
    }
    if (m_shapePreview != nullptr) {
        delete m_shapePreview;
        m_shapePreview = nullptr;
    }
}

void PreviewNode::UpdatePreview(const ImageView& view) {
    if (view.header.width != m_previewSize) {
        throw EngineError("the width={} of the image for preview should be equal previewSize={}", view.header.width, m_previewSize);
    }

    if (view.header.height != m_previewSize) {
        throw EngineError("the height={} of the image for preview should be equal previewSize={}", view.header.height, m_previewSize);
    }

    if (!m_texturePreview || (m_texSize != m_previewSize)) {
        m_texturePreview = TextureManager::Get().Create(view);
        m_texSize = m_previewSize;
    } else {
        m_texturePreview->Update(view);
    }
}

void PreviewNode::UpdatePreview(const noise::utils::Shape* sourceModule) {
    if (m_renderedPreview == nullptr) {
        m_renderedPreview = new noise::utils::RendererImage();
    }

    m_renderedPreview->SetSourceModule(sourceModule);
    m_renderedPreview->SetDestSize(m_previewSize, m_previewSize);
    m_renderedPreview->SetBounds(2.0, 6.0, 1.0, 5.0);

    UpdatePreview(m_renderedPreview->Render());
}

void PreviewNode::UpdatePreview(const noise::module::Module* sourceModule) {
    if (m_shapePreview == nullptr) {
        m_shapePreview = new utils::PlaneShape();
    }
    m_shapePreview->SetSourceModule(sourceModule);
    UpdatePreview(m_shapePreview);
}

void PreviewNode::DrawPreview() {
    if (GetIsFull()) {
        ImGui::SameLine();
        gui::Image(m_texturePreview, math::Size(m_previewSize, m_previewSize), math::Pointf(0,1), math::Pointf(1,0));
    }
}