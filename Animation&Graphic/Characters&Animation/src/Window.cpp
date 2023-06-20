#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Window.h"


// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "Model Environment";

// Objects to render
//Cube* Window::cube;

// Objects to render 
Skeleton* Window::skeleton;
Skin* Window::skin;
AnimationClip* Window::animation;

// Camera Properties
Camera* Cam;

// Interaction Variables
bool LeftDown, RightDown;
int MouseX, MouseY;

// The shader program id
GLuint Window::shaderProgram;

//float vector for DOF adjustment
std::vector<float> xfloat;
std::vector<float> yfloat;
std::vector<float> zfloat;

bool skel_in = false;
bool skin_in = false;
bool anim_in = false;
float start_time;
float t =0;
std::vector<Joint*> Pose;

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
    // cube = new Cube(glm::vec3(-1, 0, -2), glm::vec3(1, 1, 1));
    skeleton = new Skeleton();
    skeleton->Load("test.skel");
    skel_in = true;
    std::map<std::string, Joint*>::iterator it = skeleton->joints.begin();
    while (it != skeleton->joints.end()) {
        xfloat.push_back(it->second->DOFs[0]->GetValue());
        yfloat.push_back(it->second->DOFs[1]->GetValue());
        zfloat.push_back(it->second->DOFs[2]->GetValue());
        ++it;
    }
    return true;
}

bool Window::initializeObjects(const char* file) {
    std::string skelfile = "skel";
    std::string str(file);

    if (str.substr(str.length() - 4, 4).compare("skel") ==0) {
        skel_in = true;
    }
    else {
        skin_in = true;
    }

    if (skel_in) {
        skeleton = new Skeleton();
        skeleton->Load(file);
        std::map<std::string, Joint*>::iterator it = skeleton->joints.begin();
        while (it != skeleton->joints.end()) {
            xfloat.push_back(it->second->DOFs[0]->GetValue());
            yfloat.push_back(it->second->DOFs[1]->GetValue());
            zfloat.push_back(it->second->DOFs[2]->GetValue());
            ++it;
        }
    }
    else {
        skin = new Skin();
        skin->Load(file);
    }
    return true;
}

bool Window::initializeObjects(const char* file1, const char* file2) {
    skel_in = true;
    skin_in = true;
    skeleton = new Skeleton();
    skeleton->Load(file1);

    skin = new Skin();
    skin->setSkeleton(skeleton);
    skin->Load(file2);

    std::map<std::string, Joint*>::iterator it = skeleton->joints.begin();
    while (it != skeleton->joints.end()) {
        xfloat.push_back(it->second->DOFs[0]->GetValue());
        yfloat.push_back(it->second->DOFs[1]->GetValue());
        zfloat.push_back(it->second->DOFs[2]->GetValue());
        ++it;
    }

    return true;
}

bool Window::initializeObjects(const char* file1, const char* file2, const char* file3) {
    skel_in = true;
    skin_in = true;
    anim_in = true;

    skeleton = new Skeleton();
    skeleton->Load(file1);

    skin = new Skin();
    skin->setSkeleton(skeleton);
    skin->Load(file2);

    animation = new AnimationClip();
    animation->Load(file3);

    std::map<std::string, Joint*>::iterator it = skeleton->joints.begin();
    while (it != skeleton->joints.end()) {
        xfloat.push_back(it->second->DOFs[0]->GetValue());
        yfloat.push_back(it->second->DOFs[1]->GetValue());
        zfloat.push_back(it->second->DOFs[2]->GetValue());
        ++it;
    }

    it = skeleton->joints.begin();
    while (it != skeleton->joints.end()) {
        Pose.push_back(it->second);
        ++it;
    }
    start_time = glfwGetTime();
    return true;
}

void Window::cleanUp() {
    // Deallcoate the objects.
    //delete cube;
    // Deallocate the skeleton.
    if (skel_in) {
        delete skeleton;
    }
    if (skin_in) {
        delete skin;
    }
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
    if (skel_in) {
        skeleton->Update();
    }
    if (skin_in && skel_in) {
        skin->Update();
    }
    if (anim_in) {
        float currentTime = glfwGetTime();
        t += currentTime - start_time;
        start_time = currentTime;
        animation->Update( t, Pose);
        skeleton->Update();
        skin->Update();
    }
    //cube->update();
}

void Window::displayCallback(GLFWwindow* window) {
    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Control Panel");
    ImGui::Text("DOFs Adjustment");

    ImGui::SliderFloat("Time", &t, -50.0, 50.0);
    if (skel_in) {
        std::map<std::string, Joint*>::iterator it = skeleton->joints.begin();
        int i = 0;
        while (it != skeleton->joints.end()) {
            std::string name_x = it->first + "X";
            const char* str_x = name_x.c_str();
            std::string name_y = it->first + "Y";
            const char* str_y = name_y.c_str();
            std::string name_z = it->first + "Z";
            const char* str_z = name_z.c_str();

            ImGui::SliderFloat(str_x, &it->second->DOFs[0]->value, xfloat[i] - glm::pi<float>(), xfloat[i] + glm::pi<float>());
            ImGui::SliderFloat(str_y, &it->second->DOFs[1]->value, yfloat[i] - glm::pi<float>(), yfloat[i] + glm::pi<float>());
            ImGui::SliderFloat(str_z, &it->second->DOFs[2]->value, zfloat[i] - glm::pi<float>(), zfloat[i] + glm::pi<float>());
            ++it;
            ++i;
        }
    }
 

    ImGui::End();


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Render the object.
    //cube->draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    if (skel_in) {
        //skeleton->Draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    }
    if (skin_in) {
        skin->Draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
    }
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

            case GLFW_KEY_0:
                Cam->SetDistance(0.9f * Cam->GetDistance());
                break;
            case GLFW_KEY_1:
                Cam->SetDistance(1.1f * Cam->GetDistance());
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