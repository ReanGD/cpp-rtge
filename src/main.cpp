#include <chrono>
#include <iostream>

#include <imgui.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "mesh/mesh_generator.h"
#include "material/shader.h"
#include "material/texture.h"
#include "camera/fp_camera_control.h"
#include "gui/gui.h"


using defer = std::shared_ptr<void>;

std::string run() {
    bool editorMode = false;
    Window window;

    std::string err;
    if (!window.Init(false, 0.8f, err)) {
        return err;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return "Failed to initialize GLEW";
    }

    Gui gui;
    if (!gui.Init(err)) {
        return err;
    }
    gui.EnableInput(editorMode);

    glEnable(GL_DEPTH_TEST);

    Mesh plane = MeshGenerator::CreateSolidPlane(2, 2, 1.0f, 1.0f);
    Mesh cube = MeshGenerator::CreateSolidCube();
    Mesh sphere = MeshGenerator::CreateSolidSphere(30);

    auto [shader, sErr] = Shader::Create("vertex", "fragment");
    if (!sErr.empty()) {
        return sErr;
    }

    auto [shaderLight, slErr] = Shader::Create("vertex_light", "fragment_light");
    if (!slErr.empty()) {
        return slErr;
    }

    auto [texture, tErr] = Texture::Create("brice.jpg");
    if (!tErr.empty()) {
        return tErr;
    }

    auto camera = std::make_shared<Camera>(glm::quarter_pi<float>(), 0.1f, 100.0);
    camera->SetViewParams(glm::vec3(-10, 2, 0), glm::vec3(1, 0, 0));

    auto controller = std::make_shared<FPCameraControl>();
    controller->AttachCamera(camera);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    auto timeLast = std::chrono::steady_clock::now();
    while (window.StartFrame()) {
        WindowInput& wio = window.GetIO();

        if (wio.IsKeyReleasedFirstTime(Key::Escape)) {
            window.Close();
        }

        if (wio.IsKeyReleasedFirstTime(Key::F11)) {
            window.SetFullscreen(!window.IsFullscreen());
        }

        if (wio.IsKeyReleasedFirstTime(Key::F2)) {
            editorMode = !editorMode;
            window.SetCursor(editorMode ? CursorType::Arrow : CursorType::Disabled);
            gui.EnableInput(editorMode);
        }

        if (!editorMode) {
            if (wio.IsKeyStickyDown(Key::W)) {
                controller->MoveForward();
            }
            if (wio.IsKeyStickyDown(Key::S)) {
                controller->MoveBackward();
            }
            if (wio.IsKeyStickyDown(Key::A)) {
                controller->MoveLeft();
            }
            if (wio.IsKeyStickyDown(Key::D)) {
                controller->MoveRight();
            }
            float offsetX, offsetY;
            wio.GetCursorOffet(offsetX, offsetY);
            controller->Rotate(offsetX, offsetY);
        }

        uint32_t width, height;
        if (wio.GetFramebufferSize(width, height)) {
            controller->SetScreenSize(width, height);
            glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
        }

        auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - timeLast).count();
        controller->Update(deltaTime);
        gui.Update(window, deltaTime);
        timeLast = now;

        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Bind();

        glActiveTexture(GL_TEXTURE0);
        texture.Bind();

        shader.SetInt("ourTexture1", 0);

        shader.SetMat4("uProj", camera->GetProjMatrix());
        shader.SetMat4("uView", camera->GetViewMatrix());

        auto matWorld = glm::mat4(1.0f);
        matWorld = glm::translate(matWorld, glm::vec3(0, 0.51f, 0));
        shader.SetMat4("uWorld", matWorld);
        glm::mat3 matNorm = glm::inverseTranspose(glm::mat3(matWorld));
        shader.SetMat3("uNorm", matNorm);

        cube.Bind();
        cube.Draw();
        cube.Unbind();

        matWorld = glm::scale(glm::mat4(1.0), glm::vec3(20.0f));
        shader.SetMat4("uWorld", matWorld);
        matNorm = glm::inverseTranspose(glm::mat3(matWorld));
        shader.SetMat3("uNorm", matNorm);

        plane.Bind();
        plane.Draw();
        plane.Unbind();

        texture.Unbind();
        shader.Unbind();


        shaderLight.Bind();

        glActiveTexture(GL_TEXTURE0);
        texture.Bind();

        shaderLight.SetInt("uTextureDiffuse", 0);

        shaderLight.SetMat4("uProjMatrix", camera->GetProjMatrix());
        shaderLight.SetMat4("uViewMatrix", camera->GetViewMatrix());
        shaderLight.SetVec3("uToEyeDirection", camera->GetToEyeDirection());

        matWorld = glm::mat4(1.0f);
        matWorld = glm::translate(matWorld, glm::vec3(0, 0.51f, 3.0f));
        matWorld = glm::scale(matWorld, glm::vec3(1.0f, 1.0f, 2.0f));
        shaderLight.SetMat4("uWorldMatrix", matWorld);
        matNorm = glm::inverseTranspose(glm::mat3(matWorld));
        shaderLight.SetMat3("uNormalMatrix", matNorm);

        sphere.Bind();
        sphere.Draw();
        sphere.Unbind();

        matWorld = glm::mat4(1.0f);
        matWorld = glm::translate(matWorld, glm::vec3(0, 0.51f, 6.0f));
        matWorld = glm::scale(matWorld, glm::vec3(1.0f, 1.0f, 2.0f));
        shaderLight.SetMat4("uWorldMatrix", matWorld);
        matNorm = glm::mat3(matWorld);
        shaderLight.SetMat3("uNormalMatrix", matNorm);

        sphere.Bind();
        sphere.Draw();
        sphere.Unbind();

        texture.Unbind();
        shaderLight.Unbind();

        gui.NewFrame();
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Win1");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", reinterpret_cast<float*>(&clear_color)); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        gui.EndFrame();

        window.EndFrame();
    }

    texture.Delete();
    shader.Delete();
    shaderLight.Delete();
    sphere.Delete();
    cube.Delete();
    plane.Delete();

    return std::string();
}

void glfwErrorCallback(int error, const char* description) {
    std::cout << "GLFW error (" << error << "): " << description << std::endl;
}

int main() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (glfwInit() != GLFW_TRUE) {
        return EXIT_FAILURE;
    }
    defer _(nullptr, [](...){ glfwTerminate(); });

    try {
        auto err = run();
        if (!err.empty()) {
            std::cerr << "Error: " << err << std::endl;
            return EXIT_FAILURE;
        }
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
