#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Window.h"

// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "Model Environment";

// Objects to render
Cloth* Window::cloth;
Cube* Window::cube;

// Camera Properties
Camera* Cam;

// Interaction Variables
bool LeftDown, RightDown;
int MouseX, MouseY;

glm::vec3 pos = glm::vec3(0);
glm::vec3 wind = glm::vec3(0);


// The shader program id
GLuint Window::shaderProgram;

// Constructors and desctructors
bool Window::initializeProgram() {
    // Create a shader program with a vertex shader and a fragment shader.
    shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag");

    // Check the shader program.
    if (!shaderProgram) {
        std::cerr << "Failed to initialize shader program" << std::endl;
        return false;
    }

    return true;
}

bool Window::initializeObjects() {
    // Create a cube
    //cube = new Cube();
    //cloth = new Cloth(2.0f, 1.0f, 25,25 ,600, 0.5);
    cloth = new Cloth(2.0f, 1.0f, 10, 10, 750, 0.4);
    // cube = new Cube(glm::vec3(-1, 0, -2), glm::vec3(1, 1, 1));


    return true;
}

void Window::cleanUp() {
    // Deallcoate the objects.
    //delete cube;
    delete cloth;
    // Delete the shader program.
    glDeleteProgram(shaderProgram);
}

// for the Window
GLFWwindow* Window::createWindow(int width, int height) {
    // Initialize GLFW.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return NULL;
    }

    // 4x antialiasing.
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create the GLFW window.
    GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);

    // Check if the window could not be created.
    if (!window) {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        return NULL;
    }

    // Make the context of the window.
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewInit();

    // Set swap interval to 1.
    glfwSwapInterval(0);

    // set up the camera
    Cam = new Camera();
    Cam->SetAspect(float(width) / float(height));

    // initialize the interaction variables
    LeftDown = RightDown = false;
    MouseX = MouseY = 0;

    // Call the resize callback to make sure things get drawn immediately.
    Window::resizeCallback(window, width, height);

    return window;
}

void Window::resizeCallback(GLFWwindow* window, int width, int height) {
    Window::width = width;
    Window::height = height;
    // Set the viewport size.
    glViewport(0, 0, width, height);

    Cam->SetAspect(float(width) / float(height));
}

// update and draw functions
void Window::idleCallback() {
    // Perform any updates as necessary.
    Cam->Update();
    if (glfwGetTime() > 2.0) {
        for (int i = 0; i < 10; i++) {
            cloth->Update(0.0016,pos,wind);
        }
    }
    
        
    /*for (int i = 0; i < cloth->position.size(); i++) {
        std::cout << cloth->position[i].x << " " << cloth->position[i].y << " " << cloth->position[i].z << std::endl;
    }*/

    //cube->update();
}

void Window::displayCallback(GLFWwindow* window) {
    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Control Panel");
    ImGui::Text("Placement and wind adjustment");
    ImGui::SliderFloat("PosX", &(pos.x), -10.0, 10.0);
    ImGui::SliderFloat("PosY", &(pos.y), -10.0, 10.0);
    ImGui::SliderFloat("PosZ", &(pos.z), -10.0, 10.0);
    ImGui::SliderFloat("WindX", &(wind.x), -10.0, 10.0);
    ImGui::SliderFloat("WindY", &(wind.y), -10.0, 10.0);
    ImGui::SliderFloat("WindZ", &(wind.z), -10.0, 10.0);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Render the object.
    //cube->draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    cloth->Draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    // Gets events, including input such as keyboard and mouse or window resizing.
    glfwPollEvents();
    // Swap buffers.
    glfwSwapBuffers(window);
}

// helper to reset the camera
void Window::resetCamera() {
    Cam->Reset();
    Cam->SetAspect(float(Window::width) / float(Window::height));
}

// callbacks - for Interaction
void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /*
     * TODO: Modify below to add your key callbacks.
     */

    // Check for a key press.
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                // Close the window. This causes the program to also terminate.
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;

            case GLFW_KEY_R:
                resetCamera();
                break;

            default:
                break;
        }
    }
}

void Window::mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (!io.WantCaptureMouse) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            LeftDown = (action == GLFW_PRESS);
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            RightDown = (action == GLFW_PRESS);
        }
    }

}

void Window::cursor_callback(GLFWwindow* window, double currX, double currY) {
    int maxDelta = 100;
    int dx = glm::clamp((int)currX - MouseX, -maxDelta, maxDelta);
    int dy = glm::clamp(-((int)currY - MouseY), -maxDelta, maxDelta);

    MouseX = (int)currX;
    MouseY = (int)currY;

    // Move camera
    // NOTE: this should really be part of Camera::Update()
    if (LeftDown) {
        const float rate = 1.0f;
        Cam->SetAzimuth(Cam->GetAzimuth() + dx * rate);
        Cam->SetIncline(glm::clamp(Cam->GetIncline() - dy * rate, -90.0f, 90.0f));
    }
    if (RightDown) {
        const float rate = 0.005f;
        float dist = glm::clamp(Cam->GetDistance() * (1.0f - dx * rate), 0.01f, 1000.0f);
        Cam->SetDistance(dist);
    }
}