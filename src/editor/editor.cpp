#include "editor/editor.h"

#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>
#include "engine/common/glapi.h"
#include "engine/mesh/geometry_generator.h"


bool Editor::Init(Engine& engine, std::string& error) {
    if (!m_interface.Init(engine.GetGui(), error)) {
        return false;
    }
    SetEditorMode(engine, m_editorMode);

    auto& scene = engine.GetScene();

    auto camera = scene.GetCamera();
    camera->SetViewParams(glm::vec3(-10, 2, 0), glm::vec3(1, 0, 0));
    m_controller.AttachCamera(camera);

    auto shaderTex = Shader::Create("vertex_old", "fragment_tex", error);
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

    if(!m_texture.Load("brice.jpg", error)) {
        return false;
    }
    if(!m_groundTex.Load("ground.jpg", error)) {
        return false;
    }

    Material materialTex(shaderTex);
    materialTex.SetBaseTexture(0, m_texture);
    Material materialGround(shaderTex);
    materialGround.SetBaseTexture(0, m_groundTex);

    Mesh cube;
    cube.Add(GeometryGenerator::CreateSolidCube(), materialTex);
    auto matModel = glm::translate(glm::mat4(1.0f), glm::vec3(3.0, 0.51f, 0));
    cube.SetModelMatrix(matModel);
    scene.Add(cube);

    Mesh plane;
    plane.Add(GeometryGenerator::CreateSolidPlane(2, 2, 4.0f, 4.0f), materialGround);
    matModel = glm::scale(glm::mat4(1.0), glm::vec3(40.0f));
    plane.SetModelMatrix(matModel);
    scene.Add(plane);

    const auto meshCntX = 30;
    const auto meshCntZ = 30;
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

void Editor::Render(Engine& engine) {
    ProcessIO(engine);

    auto& scene = engine.GetScene();
    auto camera = scene.GetCamera();

    m_ubCamera->setUniform(m_declCamera->GetOffset("uProjMatrix"), camera->GetProjMatrix());
    m_ubCamera->setUniform(m_declCamera->GetOffset("uViewMatrix"), camera->GetViewMatrix());
    m_ubCamera->Sync();

    GLuint index = 0;
    m_ubCamera->Bind(index);

    scene.Draw();

    m_interface.Draw(engine.GetGui(), engine.GetFps(), engine.GetTpf());
}

void Editor::Destroy() {
    m_texture.Destroy();
    m_groundTex.Destroy();
}

void Editor::SetEditorMode(Engine& engine, bool value) {
    m_editorMode = value;
    engine.GetWindow().SetCursor(m_editorMode ? CursorType::Arrow : CursorType::Disabled);
    engine.GetGui().EnableInput(m_editorMode);
    m_controller.EnableInput(!m_editorMode);
}

void Editor::ProcessIO(Engine& engine) {
    auto& window = engine.GetWindow();
    WindowInput& wio = window.GetIO();

    if (wio.IsKeyReleasedFirstTime(Key::Escape)) {
        window.Close();
    }

    if (wio.IsKeyReleasedFirstTime(Key::F11)) {
        window.SetFullscreen(!window.IsFullscreen());
    }

    if (wio.IsKeyReleasedFirstTime(Key::F2)) {
        SetEditorMode(engine, !m_editorMode);
    }

    m_controller.Update(wio, engine.GetDeltaTime());
}
