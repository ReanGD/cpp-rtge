#include "editor/ui_interface.h"

#include <imgui.h>
#include <filesystem>

#include "engine/gui/widgets.h"
#include "engine/heightmap/heightmap.h"
#include "engine/common/exception.h"


static const ImGuiWindowFlags staticWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse;

static bool BeginWindow(const char* name, const math::Rectf& rect, ImGuiWindowFlags flags = staticWindowFlags) {
    bool* pOpen = nullptr;
    ImGui::SetNextWindowPos(ImVec2(rect.Left(), rect.Top()));
    ImGui::SetNextWindowSize(ImVec2(rect.Width(), rect.Height()));
    return ImGui::Begin(name, pOpen, flags);
}

UIInterface::UIInterface(Engine& engine)
    : m_engine(engine)
    , m_editor("Noise editor") {

}

void UIInterface::Init() {
    ImGuiIO& io = ImGui::GetIO();
    m_fontDefault = io.Fonts->AddFontDefault();
    if (m_fontDefault == nullptr) {
        throw EngineError("Failed to load a default font");
    }

    const auto monoFontPath = std::filesystem::current_path() / "data" / "fonts" / "NotoSans" / "NotoSans-Regular.ttf";
    m_fontMono = io.Fonts->AddFontFromFileTTF(monoFontPath.c_str(), 26, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (m_fontMono == nullptr) {
        throw EngineError("Failed to load a font from file '{}'", monoFontPath.c_str());
    }
}

void UIInterface::Render(bool editorMode) {
    auto& gui = m_engine.GetGui();
    auto& wio = m_engine.GetWindow().GetIO();
    uint32_t width, height;
    wio.GetFramebufferSize(width, height);

    gui.NewFrame();

    if (editorMode) {
        math::Rectf rect(0, 0, width, height);

        DrawRightPanel(rect.CutOffRight(300));
        DrawViewer(rect.CutOffTop(height / 2));
        DrawNodeEditor(rect);

        // ImGui::ShowDemoWindow(nullptr);
        // ImGui::ShowStyleEditor();
    } else {
        DrawInfoBar(math::Rectf(0, 0, 500, 50));
    }

    gui.EndFrame();
}

void UIInterface::Destroy() {
}

void UIInterface::DrawInfoBar(const math::Rectf& rect) {
    if (BeginWindow("infobar", rect)) {
        ImGui::PushFont(m_fontMono);
        auto pos = m_engine.GetScene().GetCamera()->GetPosition();
        auto text = fmt::format("FPS = {:.1f} TPF = {:.2f}M Pos = {:.1f}:{:.1f}:{:.1f}",
            m_engine.GetFps(),
            static_cast<double>(m_engine.GetTpf()) / 1000.0 / 1000.0,
            pos.x, pos.y, pos.z);
        ImGui::TextColored(ImColor(0xFF, 0xDA, 0x00), "%s", text.c_str());
        ImGui::PopFont();

        ImGui::End();
    }
}

void UIInterface::DrawRightPanel(const math::Rectf& rect) {
    if (BeginWindow("right_panel", rect)) {

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_None)) {
            auto camera = m_engine.GetScene().GetCamera();
            float nearPlane = camera->GetNearPlane();
            if (ImGui::DragFloat("near plane", &nearPlane, 0.1f, 0.1f, 10.0f, "%.1f")) {
                camera->SetNearPlane(nearPlane);
            }

            float farPlane = camera->GetFarPlane();
            if (ImGui::DragFloat("far plane", &farPlane, 1.0f, 10.0f, 5000.0f, "%.0f")) {
                camera->SetFarPlane(farPlane);
            }
        }

        ImGui::End();
    }
}

void UIInterface::DrawViewer(const math::Rectf& rect) {
    if (BeginWindow("viewer", rect)) {
        ImGui::End();
    }
}

void UIInterface::DrawNodeEditor(const math::Rectf& rect) {
    if (BeginWindow("node_editor", rect)) {
        m_editor.Draw();
        ImGui::End();
    }
}
