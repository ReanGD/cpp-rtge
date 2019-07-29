#include "editor/editor.h"

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>
#include "engine/common/glapi.h"
#include "engine/mesh/geometry_generator.h"


Editor::Editor(Engine& engine)
    : m_engine(engine)
    , m_interface(engine) {

}

bool Editor::Init(std::string& error) {
    if (!m_interface.Init(error)) {
        return false;
    }
    SetEditorMode(m_editorMode);

    auto& scene = m_engine.GetScene();

    auto camera = scene.GetCamera();
    camera->SetViewParams(glm::vec3(-10, 2, 0), glm::vec3(1, 0, 0));
    m_controller.AttachCamera(camera);

    auto shaderTex = Shader::Create("vertex_old", "fragment_tex", error);
    if (!shaderTex) {
        return false;
    }

    auto shaderLandscape = Shader::Create("vertex_landscape", "fragment_tex", error);
    if (!shaderTex) {
        return false;
    }

    auto shaderTexLight = Shader::Create("vertex", "fragment_tex_light", error);
    if (!shaderTexLight) {
        return false;
    }

    m_declCamera = shaderTexLight->GetUBDecl("ubCamera");
    if (!m_declCamera) {
        error = fmt::format("Сouldn't found uniform block '{}' in shader", "ubCamera");
        return false;
    }

    m_ubCamera = std::make_shared<UniformBuffer>(m_declCamera->GetSize());
    m_ubCamera->setUniform(m_declCamera->GetOffset("uProjMatrix"), camera->GetProjMatrix());
    m_ubCamera->setUniform(m_declCamera->GetOffset("uViewMatrix"), camera->GetViewMatrix());
    m_ubCamera->Sync();

    GLuint index = 0;
    glUniformBlockBinding(shaderTexLight->GetHandle(), m_declCamera->GetIndex(), index);
    m_ubCamera->Bind(index);

    auto shaderClrLight = Shader::Create("vertex_old", "fragment_clr_light", error);
    if (!shaderClrLight) {
        return false;
    }

    auto shaderLine = Shader::Create("vertex_line", "fragment_clr", error);
    if (!shaderLine) {
        return false;
    }

    auto shaderNormals = Shader::Create("vertex_normals", "geometry_normals", "fragment_normals", error);
    if (!shaderNormals) {
        return false;
    }

    if(!m_texture.Load("brice.jpg", error)) {
        return false;
    }
    if(!m_groundTex.Load("ground.jpg", error)) {
        return false;
    }
    if(!m_heightmapTex.Load("heightmap2.jpg", error)) {
        return false;
    }

    Material materialTex(shaderTex);
    materialTex.SetBaseTexture(0, m_texture);

    Material materialGround(shaderTex);
    materialGround.SetBaseTexture(0, m_groundTex);

    Material materialLandscape(shaderLandscape);
    materialLandscape.SetBaseTexture(0, m_heightmapTex);
    m_materialLine = std::make_shared<Material>(shaderLine);
    m_materialLine->SetBaseColor(glm::vec3(0.9f, 0.9f, 0.1f));

    m_materialNormals = std::make_shared<Material>(shaderNormals);
    m_materialNormals->SetBaseColor(glm::vec3(0.9f, 0.9f, 0.4f));

    Mesh cube;
    cube.Add(GeometryGenerator::CreateSolidCube(), materialTex);
    auto matModel = glm::translate(glm::mat4(1.0f), glm::vec3(3.0, 0.51f, 0));
    cube.SetModelMatrix(matModel);
    scene.Add(cube);

    Mesh plane;
    plane.Add(GeometryGenerator::CreateSolidPlane(100, 100, 1.0f, 1.0f), materialLandscape);
    matModel = glm::scale(glm::mat4(1.0), glm::vec3(100.0f, 1.0f, 100.0f));
    plane.SetModelMatrix(matModel);
    scene.Add(plane);

    m_line = GeometryGenerator::CreateLine(glm::vec3(0), glm::vec3(10.0f));

    const auto meshCntX = 1;
    const auto meshCntZ = 1;
    Material materialSphere(shaderTexLight);
    materialSphere.SetBaseColor(glm::vec3(0.6f, 0.1f, 0.1f));
    materialSphere.SetBaseTexture(0, m_groundTex);
    auto sphereGeom = GeometryGenerator::CreateSolidSphere(100);
    for(auto i=0; i!=meshCntX; ++i) {
        float posX = 1.5f * static_cast<float>(i) - static_cast<float>(meshCntX) * 1.5f / 2.0f;
        for(auto j=0; j!=meshCntZ; ++j) {
            Mesh mesh;
            mesh.Add(sphereGeom, materialSphere);

            float posZ = 2.5f * static_cast<float>(j) - static_cast<float>(meshCntZ) * 2.5f / 2.0f;
            matModel = glm::translate(glm::mat4(1.0), glm::vec3(posX, 0.51f, posZ));
            matModel = glm::scale(matModel, glm::vec3(1.0f, 1.0f, 2.0f));
            mesh.SetModelMatrix(matModel);
            scene.Add(mesh);
        }
    }

    return true;
}

void Editor::Render() {
    ProcessIO();

    auto& scene = m_engine.GetScene();
    auto camera = scene.GetCamera();

    m_ubCamera->setUniform(m_declCamera->GetOffset("uProjMatrix"), camera->GetProjMatrix());
    m_ubCamera->setUniform(m_declCamera->GetOffset("uViewMatrix"), camera->GetViewMatrix());
    m_ubCamera->Sync();

    GLuint index = 0;
    m_ubCamera->Bind(index);

    scene.Draw();
    if (m_showNormals) {
        scene.DrawWithMaterial(*m_materialNormals);
    }

    glm::mat4 matModel(1.0f);
    m_materialLine->Bind(camera, matModel, matModel);
    m_line->Bind();
    m_line->Draw();
    m_line->Unbind();
    m_materialLine->Unbind();

    m_interface.Render(m_editorMode);
}

void Editor::Destroy() {
    m_interface.Destroy();

    m_texture.Destroy();
    m_groundTex.Destroy();
    m_heightmapTex.Destroy();

    m_materialLine.reset();
    m_materialNormals.reset();
    m_line.reset();
}

void Editor::SetEditorMode(bool value) {
    m_editorMode = value;
    m_engine.GetWindow().SetCursor(m_editorMode ? CursorType::Arrow : CursorType::Disabled);
    m_engine.GetGui().EnableInput(m_editorMode);
    m_controller.EnableInput(!m_editorMode);
}

void Editor::ProcessIO() {
    auto& window = m_engine.GetWindow();
    WindowInput& wio = window.GetIO();

    if (wio.IsKeyReleasedFirstTime(Key::Escape)) {
        window.Close();
    }

    if (wio.IsKeyReleasedFirstTime(Key::F11)) {
        window.SetFullscreen(!window.IsFullscreen());
    }

    if (wio.IsKeyReleasedFirstTime(Key::F2)) {
        SetEditorMode(!m_editorMode);
    }

    if (wio.IsKeyReleasedFirstTime(Key::L)) {
        m_engine.SetFillPoligone(!m_engine.IsFillPoligone());
    }

    if (wio.IsKeyReleasedFirstTime(Key::N)) {
        m_showNormals = !m_showNormals;
    }

    if (m_editorMode) {
        if (wio.IsKeyReleasedFirstTime(Key::MouseLeft)) {
            glm::vec2 coord;
            wio.GetHomogeneousClipCursorPosition(coord.x, coord.y);
            auto camera = m_engine.GetScene().GetCamera();
            glm::vec3 pos = camera->GetPosition();
            glm::vec3 rayWorld = camera->HomogeneousPositionToRay(coord);

            auto& vertexBuffer = m_line->GetVertexBuffer();
            VertexP* vb = static_cast<VertexP*>(vertexBuffer.Lock());
            vb[0].Position	= pos;
            vb[1].Position	= pos + rayWorld * 10.0f;
            if (!vertexBuffer.Unlock()) {
                throw std::runtime_error("Can't unlock vertex buffer in GeometryGenerator::CreateLine");
            }
        }
    }

    m_controller.Update(wio, m_engine.GetDeltaTime());
}
