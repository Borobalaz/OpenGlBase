#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include "Gui/GuiLayer.h"
#include "Gui/Components/SliderComponent.h"
#include "Scene.h"
#include "FpsCameraMovement.h"

int main()
{
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    GuiLayer guiLayer;
    Scene scene;
    scene.Init();
    scene.SetSkybox(SkyboxFaces{
        "assets/textures/skybox/right.png",
        "assets/textures/skybox/left.png",
        "assets/textures/skybox/top.png",
        "assets/textures/skybox/bottom.png",
        "assets/textures/skybox/front.png",
        "assets/textures/skybox/back.png"
    });

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    //scene.GetCamera().SetMoveComponent(
    //    std::make_unique<FpsCameraMovement>(window)
    //);

    std::shared_ptr<SliderComponent> matrixTestUniformSlider =
        std::make_shared<SliderComponent>("Matrix Test Uniform", 0.0f, 2.0f, scene.GetMatrixTestUniform());
    matrixTestUniformSlider->SetFrame(16.0f, 16.0f, 220.0f);
    matrixTestUniformSlider->SetOnValueChanged(
        [&scene](float newValue)
        {
            scene.SetMatrixTestUniform(newValue);
        });
    guiLayer.PutComponent(matrixTestUniformSlider);

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        double cursorXWindow = 0.0;
        double cursorYWindow = 0.0;
        glfwGetCursorPos(window, &cursorXWindow, &cursorYWindow);

        const float scaleX = (windowWidth > 0) ? (static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth)) : 1.0f;
        const float scaleY = (windowHeight > 0) ? (static_cast<float>(framebufferHeight) / static_cast<float>(windowHeight)) : 1.0f;
        const float pointerX = static_cast<float>(cursorXWindow) * scaleX;
        const float pointerY = static_cast<float>(cursorYWindow) * scaleY;
        const bool pointerDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        guiLayer.BeginFrame(framebufferWidth, framebufferHeight);
        guiLayer.SetPointerState(pointerX, pointerY, pointerDown);
        scene.SetCameraAspect(guiLayer.GetSceneAspect());

        scene.Update(deltaTime);

        guiLayer.RenderSidebar();
        guiLayer.BeginScenePass();
        scene.Render();
        guiLayer.EndFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    scene.Destroy();
    glfwTerminate();
}