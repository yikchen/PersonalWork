#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Window.h"

// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "Model Environment";

// Objects to render
Cube* Window::cube;
Cube* Window::c0;
Cube* Window::c1;
Cube* Window::c2;
Cube* Window::c3;
Cube* Window::c4;

Quaternion* Window::starting_p;

float c0_x;
float c0_y;
float c0_z;

float c1_x;
float c1_y;
float c1_z;

float c2_x;
float c2_y;
float c2_z;

float c3_x;
float c3_y;
float c3_z;

float c4_x;
float c4_y;
float c4_z;

float start_t;

bool startSlerp;
bool startCatmullRom;

// Camera Properties
Camera* Cam;

// Interaction Variables
bool LeftDown, RightDown;
int MouseX, MouseY;

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
    cube = new Cube();
    cube->q->setTranslation(glm::vec3(-6, 0, 0));
    cube->color = glm::vec3(0.1f, 0.95, 1.0);

    starting_p = new Quaternion();
    starting_p->setTranslation(glm::vec3(-6, 0, 0));

    c0 = new Cube();
    c0->q->setTranslation(glm::vec3(-4, 0, 0));
    c1 = new Cube();
    c1->q->setTranslation(glm::vec3(-2, 0, 0));
    c2 = new Cube();
    c2->q->setTranslation(glm::vec3(0, 0, 0));
    c3 = new Cube();
    c3->q->setTranslation(glm::vec3(2, 0, 0));
    c4 = new Cube();
    c4->q->setTranslation(glm::vec3(4, 0, 0));

    startSlerp = false;
    startCatmullRom = false;
    // cube = new Cube(glm::vec3(-1, 0, -2), glm::vec3(1, 1, 1));

    return true;
}

void Window::cleanUp() {
    // Deallcoate the objects.
    delete cube;
    delete c0;
    delete c1;
    delete c2;
    delete c3;
    delete c4;

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

    float t = glfwGetTime() / 2.0 - start_t;
    //X,Y,Z 

    if (startSlerp && t <= 1) {
        float dot = cube->q->Dot(*starting_p, *c0->q);
        if (dot < 0) {
            c0->q->negate();
        }
        cube->update(t, *starting_p, *c0->q);
    }
    else if (startSlerp && t > 1 && t <= 2) {
        float dot = cube->q->Dot(*c0->q, *c1->q);
        if (dot < 0) {
            c1->q->negate();
        }
        cube->update(t-1, *c0->q, *c1->q);
    }
    else if (startSlerp && t > 2 && t <= 3) {
        float dot = cube->q->Dot(*c1->q, *c2->q);
        if (dot < 0) {
            c2->q->negate();
        }
        cube->update(t-2, *c1->q, *c2->q);
    }
    else if (startSlerp && t > 3 && t <= 4) {
        float dot = cube->q->Dot(*c2->q, *c3->q);
        if (dot < 0) {
            c3->q->negate();
        }
        cube->update(t-3, *c2->q, *c3->q);
    }
    else if (startSlerp && t > 4 && t <= 5) {
        float dot = cube->q->Dot(*c3->q, *c4->q);
        if (dot < 0) {
            c4->q->negate();
        }
        cube->update(t-4, *c3->q, *c4->q);
    }

    if (startCatmullRom && t <= 1) {
        cube->update(t, *starting_p, *starting_p, *c0->q, *c1->q);
    }
    else if (startCatmullRom && t > 1 && t <= 2) {
        cube->update(t-1, *starting_p, *c0->q, *c1->q, *c2->q);
    }
    else if (startCatmullRom && t > 2 && t <= 3) {
        cube->update(t - 2, *c0->q,*c1->q, *c2->q, *c3->q);
    }
    else if (startCatmullRom && t > 3 && t <= 4) {
        cube->update(t - 3,*c1->q, *c2->q, *c3->q, *c4->q);
    }
    else if (startCatmullRom && t > 4 && t <= 5) {
        cube->update(t - 4, *c2->q,*c3->q, *c4->q, *c4->q);
    }

    c0->update(c0_x, c0_y, c0_z);
    c1->update(c1_x, c1_y, c1_z);
    c2->update(c2_x, c2_y, c2_z);
    c3->update(c3_x, c3_y, c3_z);
    c4->update(c4_x, c4_y, c4_z);
    cube->update();
}

void Window::displayCallback(GLFWwindow* window) {
    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the object.
    cube->draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    c0->drawFrame(Cam->GetViewProjectMtx(), Window::shaderProgram);
    c1->drawFrame(Cam->GetViewProjectMtx(), Window::shaderProgram);
    c2->drawFrame(Cam->GetViewProjectMtx(), Window::shaderProgram);
    c3->drawFrame(Cam->GetViewProjectMtx(), Window::shaderProgram);
    c4->drawFrame(Cam->GetViewProjectMtx(), Window::shaderProgram);

    ImGui::Begin("Control Panel");
    ImGui::Text("Rotation of Cube");

    if (ImGui::Button("Start Slerp")) {
        cube->q->qua = starting_p->qua;
        start_t = glfwGetTime() / 2.0;
        startSlerp = true;
        startCatmullRom = false;
    }

    if (ImGui::Button("Start CatmullRom")) {
        start_t = glfwGetTime() / 2.0;
        startSlerp = false;
        startCatmullRom = true;
        float dot = cube->q->Dot(*starting_p, *c0->q);
        if (dot < 0) {
            c0->q->negate();
        }
        dot = cube->q->Dot(*c0->q, *c1->q);
        if (dot < 0) {
            c1->q->negate();
        }
        dot = cube->q->Dot(*c1->q, *c2->q);
        if (dot < 0) {
            c2->q->negate();
        }
        dot = cube->q->Dot(*c2->q, *c3->q);
        if (dot < 0) {
            c3->q->negate();
        }
        dot = cube->q->Dot(*c3->q, *c4->q);
        if (dot < 0) {
            c4->q->negate();
        }

    }

    ImGui::SliderFloat("cube_0_X", &(c0_x), -360.0, 360.0);
    ImGui::SliderFloat("cube_0_Y", &(c0_y), -360.0, 360.0);
    ImGui::SliderFloat("cube_0_Z", &(c0_z), -360.0, 360.0);

    ImGui::SliderFloat("cube_1_X", &(c1_x), -360.0, 360.0);
    ImGui::SliderFloat("cube_1_Y", &(c1_y), -360.0, 360.0);
    ImGui::SliderFloat("cube_1_Z", &(c1_z), -360.0, 360.0);

    ImGui::SliderFloat("cube_2_X", &(c2_x), -360.0, 360.0);
    ImGui::SliderFloat("cube_2_Y", &(c2_y), -360.0, 360.0);
    ImGui::SliderFloat("cube_2_Z", &(c2_z), -360.0, 360.0);

    ImGui::SliderFloat("cube_3_X", &(c3_x), -360.0, 360.0);
    ImGui::SliderFloat("cube_3_Y", &(c3_y), -360.0, 360.0);
    ImGui::SliderFloat("cube_3_Z", &(c3_z), -360.0, 360.0);

    ImGui::SliderFloat("cube_4_X", &(c4_x), -360.0, 360.0);
    ImGui::SliderFloat("cube_4_Y", &(c4_y), -360.0, 360.0);
    ImGui::SliderFloat("cube_4_Z", &(c4_z), -360.0, 360.0);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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