#include "middleware/node_editor/node_editor.h"

#include "middleware/node_editor/noise_node.h"
#include "middleware/node_editor/node_editor_storage.h"


namespace ne = ax::NodeEditor;

NodeEditor::NodeEditor(const std::string& name)
    : m_name(name) {
    m_context = ne::CreateEditor();
    m_storage = std::make_shared<NodeEditorStorage>();
}

NodeEditor::~NodeEditor() {
    if (m_storage) {
        m_storage.reset();
    }

    if (m_context) {
        ne::DestroyEditor(m_context);
        m_context = nullptr;
    }
}

void NodeEditor::Draw() {
    ne::SetCurrentEditor(m_context);
    ne::Begin(m_name.c_str());

    m_storage->Draw();

    if (ne::BeginCreate()) {
        ne::PinId pinIdFirst, pinIdSecond;
        if (ne::QueryNewLink(&pinIdFirst, &pinIdSecond)) {
            bool checkOnly = true;
            if (!m_storage->AddLink(pinIdFirst, pinIdSecond, checkOnly)) {
                ne::RejectNewItem();
            } else if (ne::AcceptNewItem()) {
                checkOnly = false;
                m_storage->AddLink(pinIdFirst, pinIdSecond, checkOnly);
            }
        }
        ne::EndCreate();
    }

    if (ne::BeginDelete()) {
        ne::LinkId linkId;
        while (ne::QueryDeletedLink(&linkId)) {
            bool checkOnly = true;
            if (!m_storage->DelLink(linkId, checkOnly)) {
                ne::RejectDeletedItem();
            } else if (ne::AcceptDeletedItem()) {
                checkOnly = false;
                m_storage->DelLink(linkId, checkOnly);
            }
        }
        ne::EndDelete();
    }

    auto currentCursorPosition = ImGui::GetMousePos();
    static auto openPopupPosition = ImGui::GetMousePos();
    ne::Suspend();
    if (ne::ShowBackgroundContextMenu()) {
        ImGui::OpenPopup("Create New Node");
        openPopupPosition = currentCursorPosition;
    }
    ne::Resume();

    ne::Suspend();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

    if (ImGui::BeginPopup("Create New Node")) {
        std::shared_ptr<BaseNode> node = nullptr;

        if (ImGui::BeginMenu("Generator")) {
            if (ImGui::MenuItem("Billow")) {
                node = std::make_shared<BillowNode>();
            } else if (ImGui::MenuItem("Checkerboard")) {
                node = std::make_shared<CheckerboardNode>();
            } else if (ImGui::MenuItem("Const")) {
                node = std::make_shared<ConstNode>();
            } else if (ImGui::MenuItem("Cylinders")) {
                node = std::make_shared<CylindersNode>();
            } else if (ImGui::MenuItem("Perlin")) {
                node = std::make_shared<PerlinNode>();
            } else if (ImGui::MenuItem("RidgedMulti")) {
                node = std::make_shared<RidgedMultiNode>();
            } else if (ImGui::MenuItem("Spheres")) {
                node = std::make_shared<SpheresNode>();
            } else if (ImGui::MenuItem("Voronoi")) {
                node = std::make_shared<VoronoiNode>();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Modifier")) {
            if (ImGui::MenuItem("Abs")) {
                node = std::make_shared<AbsNode>();
            } else if (ImGui::MenuItem("Clamp")) {
                node = std::make_shared<ClampNode>();
            } else if (ImGui::MenuItem("Exponent")) {
                node = std::make_shared<ExponentNode>();
            } else if (ImGui::MenuItem("Invert")) {
                node = std::make_shared<InvertNode>();
            } else if (ImGui::MenuItem("ScaleBias")) {
                node = std::make_shared<ScaleBiasNode>();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Combiner")) {
            if (ImGui::MenuItem("Add")) {
                node = std::make_shared<AddNode>();
            } else if (ImGui::MenuItem("Max")) {
                node = std::make_shared<MaxNode>();
            } else if (ImGui::MenuItem("Min")) {
                node = std::make_shared<MinNode>();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Selector")) {
            if (ImGui::MenuItem("Select")) {
                node = std::make_shared<SelectNode>();
            }
            ImGui::EndMenu();
        }

        if (node) {
            m_storage->AddNode(node);
            ne::SetNodePosition(ne::NodeId(node.get()), openPopupPosition);
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
    ne::Resume();

    ne::End();
}