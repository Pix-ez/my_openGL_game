#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <glad/glad.h>
#include <cmath>
#include <string>
#include <format>
#include <list>
#include <core/Shader.h>
#include <core/camera.h>
#include <core/inputController.h>
// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
#include <core/utils.h>
#include <core/Model.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "GameObject.h"
#include "LightManager.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Primitives.h"
#include "ProjectManager.h"
#include "ResourceManager.h"
#include "tinyfiledialogs.h"
// #include <assimp/DefaultLogger.hpp>

//settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

InputController inputController;

// A simple struct to hold notification data
struct Notification {
    std::string message;
    float timer; // How long it's been visible
};

// A global or member variable in your Application class
std::list<Notification> g_notifications; // Use a list for easy removal

// A function to add a new notification
void AddNotification(const std::string& message) {
    g_notifications.push_back({message, 0.0f});
}

void RenderNotifications() {
    // Get the main viewport to position the notifications
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos; // Top-left corner of the window
    ImVec2 work_size = viewport->WorkSize;

    // Position for the next notification
    ImVec2 notification_pos = {work_pos.x + work_size.x - 10, work_pos.y + work_size.y - 10};

    // Use a window with no background, no title bar, no input, etc.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;

    float padding = 10.0f;
    
    // Iterate through notifications in reverse to draw newest on top
    for (auto it = g_notifications.rbegin(); it != g_notifications.rend(); ) {
        Notification& notification = *it;
        
        // Update timer
        notification.timer += ImGui::GetIO().DeltaTime;
        
        // Fade out after a few seconds
        float opacity = 1.0f;
        if (notification.timer > 3.0f) {
            opacity = std::max(0.0f, 1.0f - (notification.timer - 3.0f) / 2.0f);
        }
        
        if (opacity <= 0.0f) {
            // Remove the notification
            it = std::list<Notification>::reverse_iterator(g_notifications.erase(std::next(it).base()));
            continue;
        }

        ImGui::SetNextWindowBgAlpha(0.7f * opacity);
        ImGui::SetNextWindowPos(notification_pos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        
        std::string window_name = "Notification##" + notification.message; // Unique ID
        ImGui::Begin(window_name.c_str(), nullptr, window_flags);
        ImGui::TextColored({0.8f, 1.0f, 0.8f, opacity}, "✔"); // Green checkmark
        ImGui::SameLine();
        ImGui::TextColored({1.0f, 1.0f, 1.0f, opacity}, "%s", notification.message.c_str());
        ImGui::End();

        // Move the position for the next notification up
        notification_pos.y -= ImGui::GetItemRectSize().y + padding;

        ++it;
    }
}


// For the shadow pass
void RenderSceneGeometryOnly(Shader& shader, const std::vector<std::shared_ptr<GameObject>>& sceneRoots) {
    for (const auto& obj : sceneRoots) {
        obj->DrawGeometryOnly(shader);
    }
}

// For the main color pass
void RenderSceneWithMaterial(Shader& shader, const std::vector<std::shared_ptr<GameObject>>& sceneRoots, int& textureUnit) {
    for (const auto& obj : sceneRoots) {
        obj->DrawWithMaterial(shader, textureUnit);
    }
}

void RemoveObjectFromScene(std::shared_ptr<GameObject> object, std::vector<std::shared_ptr<GameObject>>& roots) {
    if (!object) return;

    // First, check if the object has a parent
    if (auto parent_ptr = object->GetParent().lock()) {
        // If it has a parent, tell the parent to remove it from its children list
        parent_ptr->RemoveChild(object);
    } else {
        // If it has no parent, it must be a root object. Remove it from the roots list.
        // The "erase-remove idiom" is the standard way to do this in C++.
        roots.erase(std::remove(roots.begin(), roots.end(), object), roots.end());
    }
}

 // A simple struct to hold the state we want to save/restore
    struct GameObjectState {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };

    // State variables for the editor
    bool m_isPlaying = false;
    std::unordered_map<uint32_t, GameObjectState> m_editorStateSnapshot;

    // Create the scene list (only for root objects)
    ProjectManager projectManager;
    std::vector<std::shared_ptr<GameObject>> sceneRoots;

// Traverses the hierarchy from a given node and saves its state
void SaveStateRecursive(std::shared_ptr<GameObject> go, std::unordered_map<uint32_t, GameObjectState>& snapshot) {
    if (!go) return;

    GameObjectState state;
    state.position = go->position;
    state.rotation = go->rotation;
    state.scale = go->scale;
    snapshot[go->GetID()] = state;

    for (const auto& child : go->GetChildren()) {
        SaveStateRecursive(child, snapshot);
    }
}

// Traverses the hierarchy and restores state from the snapshot
void RestoreStateRecursive(std::shared_ptr<GameObject> go, const std::unordered_map<uint32_t, GameObjectState>& snapshot) {
    if (!go) return;

    auto it = snapshot.find(go->GetID());
    if (it != snapshot.end()) {
        const GameObjectState& state = it->second;
        go->position = state.position;
        go->rotation = state.rotation;
        go->scale = state.scale;

        // Also reset runtime physics properties to a neutral state
        if (go->physics) {
            go->physics->velocity = glm::vec3(0.0f);
            go->physics->acceleration = glm::vec3(0.0f);
        }
    }

    for (const auto& child : go->GetChildren()) {
        RestoreStateRecursive(child, snapshot);
    }
}

void GetAllColliders(std::shared_ptr<GameObject> go, std::vector<ColliderComponent*>& colliders) {
    if (!go) return;

    // Add this object's collider to the list if it exists
    if (go->collider) {
        colliders.push_back(go->collider.get());
    }

    // Recursively call for all children
    for (const auto& child : go->GetChildren()) {
        GetAllColliders(child, colliders);
    }
}

// --- Main Control Functions ---

void StartSimulation() {
    if (m_isPlaying) return;

    m_editorStateSnapshot.clear();
    // Iterate through all root objects and recursively save their state
    for (const auto& root : sceneRoots) {
        SaveStateRecursive(root, m_editorStateSnapshot);
    }

    m_isPlaying = true;
    printf("Simulation Started.\n");
}

void StopSimulation() {
    if (!m_isPlaying) return;

    // Iterate through all root objects and recursively restore their state
    for (const auto& root : sceneRoots) {
        RestoreStateRecursive(root, m_editorStateSnapshot);
    }

    m_isPlaying = false;
    printf("Simulation Stopped.\n");
}
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
int main(int argc, char *argv[]){

    // Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
    // Assimp::LogStream* stderrStream = Assimp::LogStream::createDefaultStream(aiDefaultLogStream_STDERR);
    // Assimp::DefaultLogger::get()->attachStream(stderrStream, Assimp::Logger::NORMAL | Assimp::Logger::DEBUGGING | Assimp::Logger::VERBOSE);


    // --- GET THE BASE PATH TO THE EXECUTABLE ---
    const char* basePath_c = SDL_GetBasePath();
    if (!basePath_c) {
        std::cerr << "Error getting base path: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::string basePath(basePath_c);
    //path for testing off when release mode
    basePath =basePath + "../../";

    std::cout<< basePath.c_str() <<"\n";


    if(!SDL_Init(SDL_INIT_VIDEO)){
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN ;

    SDL_Window* window = SDL_CreateWindow("My OpenGl APP",
                                        (int)(1280 * main_scale), 
                                        (int)(720 * main_scale),
                                        windowFlags);

    if(!window){
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if(!glContext){
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if(!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)){
        std::cerr << "Failed to initialize GLAD" << std::endl;
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    std::cout << "OpenGL Loaded" << std::endl;
    std::cout << "Vendor:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version:  " << glGetString(GL_VERSION) << std::endl;

    //set window color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    Uint64 prevCounter = SDL_GetPerformanceCounter();
    Uint64 fpsCounter = 0;
    double accumulatedTime = 0.0;
    const double fpsUpdateInterval = 0.5; // seconds
    const char* baseTitle = "My OpenGL APP";


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    // Setup scaling
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale; 
    
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float rotation_angle = 2.0f;
    float lightIntensity = 1.0f;
    float emission_strength = 1.0f;
    glm::vec3 pointlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 spotlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    // lighting
    // glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f); 

    
    bool enable_mouse = false;  // false = gameplay mode
    bool last_mouse_enabled = true; // force initial mode setup
    bool escapePressed = false;
    bool was_mouse_enabled_last_frame = true;

    // // ca
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); 
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);


  
    //user our custom shader
    std::string vertPath = basePath + "shader/shader.vert";
    std::string fragPath = basePath + "shader/shader.frag";

    std::string depth_vert = basePath + "shader/shadow_mapping.vert";
    std::string depth_frag = basePath + "shader/shadow_mapping.frag";

    std::string quad_vert = basePath + "shader/debug_quad.vert";
    std::string quad_frag = basePath + "shader/debug_quad.frag";
   


    // std::string lightCube_vert = basePath + "shader/light_cube.vert";
    // std::string lightCube_frag = basePath + "shader/light_cube.frag";
    // auto lightCubeShader = std::make_shared<Shader>(lightCube_vert, lightCube_frag);
    

    // 1. Create a single instance of the manager
    LightManager lightManager;
    // 2. Tell the base Light class about it
    Light::SetLightManager(&lightManager);

   
    std::weak_ptr<GameObject> selectedObject;
    bool isSceneDirty = false;
bool showExitPopup = false;
 
    std::shared_ptr<GameObject> objectToDelete = nullptr; // A pointer to hold the object we want to delete

    auto mainShader = std::make_shared<Shader>(vertPath, fragPath);
    auto depthShader = std::make_shared<Shader>(depth_vert, depth_frag);
    auto debugQuadShader = std::make_shared<Shader>(quad_vert, quad_frag);

    // auto backpackModel = std::make_shared<Model>((basePath + "resources/models/backpack/backpack.obj"), mainShader);

    // auto suzanModel = std::make_shared<Model>((basePath + "resources/models/suzan/blender_monkey.fbx"), mainShader);
    // auto suzan =  std::make_shared<GameObject>(suzanModel, "suzan");

    auto bcubeModel = std::make_shared<Model>((basePath + "resources/models/suzan/cube.fbx"));
    auto cube =  std::make_shared<GameObject>(bcubeModel, "cube");
    sceneRoots.push_back(cube);
    // auto backpack =  std::make_shared<GameObject>(backpackModel, "Backpack");
    // backpack->position = {-2.0f, 4.0f, -1.0f};
    // backpack->scale = {1,1,1};
    // sceneRoots.push_back(backpack);


    // suzan->position = {-2.0f, 4.0f, -1.0f};
    // suzan->scale = {1,1,1};
    // sceneRoots.push_back(suzan);

    // --- Create a Cube ---
std::vector<Vertex> cubeVertices;
std::vector<unsigned int> cubeIndices;
Primitives::GenerateCube(cubeVertices, cubeIndices); // Fill the vectors
auto cubeModel = std::make_shared<Model>(cubeVertices, cubeIndices); // Create a model from the data
auto cubeObject = std::make_shared<GameObject>(cubeModel, "MyCube");
cubeObject->position = {2.0f, 0.5f, 0.0f};
sceneRoots.push_back(cubeObject);

// std::vector<Vertex> cubeVertices;
// std::vector<unsigned int> cubeIndices;
// Primitives::GenerateCube(cubeVertices, cubeIndices); // Fill the vectors
auto cubeModel2 = std::make_shared<Model>(cubeVertices, cubeIndices); // Create a model from the data
auto cubeObject2 = std::make_shared<GameObject>(cubeModel2, "MyCube2");
cubeObject2->position = {2.0f, 0.5f, 0.0f};
sceneRoots.push_back(cubeObject2);



// // --- Create a Plane ---
std::vector<Vertex> planeVertices;
std::vector<unsigned int> planeIndices;
Primitives::GeneratePlane(planeVertices, planeIndices);
auto planeModel = std::make_shared<Model>(planeVertices, planeIndices);
auto planeObject = std::make_shared<GameObject>(planeModel, "GroundPlane");
planeObject->position = {0.0f, -0.5f, 0.0f};
planeObject->scale = {10.0f, 1.0f, 10.0f};
sceneRoots.push_back(planeObject);
    
    // auto sponzaModel = std::make_shared<Model>((basePath + "resources/models/Sponza/sponza.obj"), mainShader);

    // auto sponza =  std::make_shared<GameObject>(sponzaModel, "Sponza");
    // sponza->position = {0,0,0};
    // sponza->scale = {1,1,1};
    // sceneRoots.push_back(sponza);


    // 3. Create your lights just like any other GameObject
    auto dirLight = std::make_shared<DirectionalLight>();
    dirLight->position= {-2.0f, 4.0f, -1.0f};
    dirLight->rotation = {-20.0f, -30.0f, 0.0f}; // Set direction via rotation
    // dirLight->diffuse = {0.4f, 0.4f, 0.4f};
    dirLight->ambient = {1.0f, 1.0f, 1.0f};
    dirLight->diffuse = {1.0f, 1.0f, 1.0f};
    dirLight->specular = {1.0f, 1.0f, 1.0f};
    sceneRoots.push_back(dirLight); // Add to scene to be updated/drawn
    lightManager.RegisterLight(dirLight.get());

    // auto pointLight1 = std::make_shared<PointLight>(suzan->model); // Use a cube model
    // pointLight1->position = {0.7f, 2.0f, 2.0f};
    // pointLight1->diffuse = {1.0f, 0.0f, 0.0f}; // A red light
    // sceneRoots.push_back(pointLight1);

    // lightManager.RegisterLight(pointLight1.get());

    // auto spotLight1 = std::make_shared<SpotLight>(suzan->model); // Use a cube model
    // spotLight1->position = {0.7f, 2.0f, 2.0f};
    // spotLight1->diffuse = {1.0f, 0.0f, 0.0f}; // A red light
    // sceneRoots.push_back(spotLight1);

    // lightManager.RegisterLight(spotLight1.get());
    
    // float vertices[] = {
    //     //vertex pos          vertex normals        texture coords
    //     -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,   
    //      0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,   
    //      0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   
    //      0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   
    //     -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 1.0f,   
    //     -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,   

    //     -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,   
    //      0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,   
    //      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   
    //      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   
    //     -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,   
    //     -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,   

    //     -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   
    //     -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   
    //     -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   
    //     -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   
    //     -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   
    //     -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   

    //      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   
    //      0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   
    //      0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   
    //      0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   
    //      0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   
    //      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   

    //     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,   
    //     0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
    //     0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    //     0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    //     -0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    //     -0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    //     -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    //     0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    //     0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    //     0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    //     -0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    //     -0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    // };
    // // // world space positions of our cubes
    // glm::vec3 cubePositions[] = {
    //     glm::vec3( 0.0f,  0.0f,  0.0f),
    //     glm::vec3( 2.0f,  5.0f, -15.0f),
    //     glm::vec3(-1.5f, -2.2f, -2.5f),
    //     glm::vec3(-3.8f, -2.0f, -12.3f),
    //     glm::vec3( 2.4f, -0.4f, -3.5f),
    //     glm::vec3(-1.7f,  3.0f, -7.5f),
    //     glm::vec3( 1.3f, -2.0f, -2.5f),
    //     glm::vec3( 1.5f,  2.0f, -2.5f),
    //     glm::vec3( 1.5f,  0.2f, -1.5f),
    //     glm::vec3(-1.3f,  1.0f, -1.5f)
    // };

    // positions of the point lights
    // glm::vec3 pointLightPositions[] = {
    //     glm::vec3( 0.7f,  0.2f,  2.0f),
    //     glm::vec3( 2.3f, -3.3f, -4.0f),
    //     glm::vec3(-4.0f,  2.0f, -12.0f),
    //     glm::vec3( 0.0f,  0.0f, -3.0f)
    // };
    // unsigned int VBO, VAO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);

    
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // glBindVertexArray(VAO);
    // // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // // normal attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);
    // // texture coord attribute
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray(2);


    // // // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    // unsigned int lightCubeVAO;
    // glGenVertexArrays(1, &lightCubeVAO);
    // glBindVertexArray(lightCubeVAO);

    // // // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // unsigned int diffuseMap = loadTexture((basePath + "textures/container2.png").c_str());
    // unsigned int specularMap = loadTexture((basePath + "textures/container2_specular.png").c_str());
    // unsigned int emissionMap = loadTexture((basePath + "textures/matrix.jpg").c_str());
    
    // plane
    // float planeVertices[] = {
    //     // positions            // normals         // texcoords
    //      25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    //     -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    //     -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

    //      25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    //     -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
    //      25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    // };
    // // plane VAO
    // unsigned int planeVBO, planeVAO;
    // glGenVertexArrays(1, &planeVAO);
    // glGenBuffers(1, &planeVBO);
    // glBindVertexArray(planeVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(2);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glBindVertexArray(0);

    // // // load textures
    // // // -------------
    // unsigned int woodTexture = loadTexture((basePath+"resources/textures/wood.png"), false);



    // create buffer for depth 
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    //create texture to store depth
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);      
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    mainShader->setInt("shadowMap", 1);
    debugQuadShader->use();
    debugQuadShader->setInt("depthMap",0);

    //to enable wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bool is_running = true;
    

    
    SDL_Event event;
    while(is_running){
        escapePressed = false;

        Uint64 currentCounter = SDL_GetPerformanceCounter();
        Uint64 deltaCounter = currentCounter - prevCounter;
        prevCounter = currentCounter;

        double deltaTime = double(deltaCounter) / SDL_GetPerformanceFrequency();
        accumulatedTime += deltaTime;
        fpsCounter++;
        float cameraSpeed = static_cast<float>(2.5 * deltaTime);

        // Update FPS display every fpsUpdateInterval seconds
        if (accumulatedTime >= fpsUpdateInterval) {
            double fps = fpsCounter / accumulatedTime;
            std::string title = std::format("{} - FPS: {:.1f}", baseTitle, fps);
            SDL_SetWindowTitle(window, title.c_str());

            accumulatedTime = 0.0;
            fpsCounter = 0;
        }
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_EVENT_QUIT) {
                if (isSceneDirty) {
                    showExitPopup = true;
                } else {
                    is_running = false;
                }
            }

            inputController.BeginFrame();

            inputController.ProcessEvent(event);

            if(event.type == SDL_EVENT_QUIT) is_running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) is_running = false;

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.repeat == 0) {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    escapePressed = true;
                }
            }
        ImGui_ImplSDL3_ProcessEvent(&event); // Forward your event to backend
        }

        if (escapePressed) {
            enable_mouse = !enable_mouse;
            std::cout << "ESC pressed. enable_mouse = " << enable_mouse << "\n";
        }
        
        if (enable_mouse != last_mouse_enabled) {
            if (enable_mouse) {
                SDL_ShowCursor();
                SDL_SetWindowRelativeMouseMode(window, false);
                firstMouse = true;
            } else {
                SDL_HideCursor();
                SDL_SetWindowRelativeMouseMode(window, true);
                SDL_GetRelativeMouseState(nullptr, nullptr); // flush input buffer
            }
            last_mouse_enabled = enable_mouse;
        }
            
        if (!enable_mouse) {
            // Keyboard
            if (inputController.IsKeyDown(SDL_SCANCODE_W)) camera.ProcessKeyboard(FORWARD, deltaTime);
            if (inputController.IsKeyDown(SDL_SCANCODE_A)) camera.ProcessKeyboard(LEFT, deltaTime);
            if (inputController.IsKeyDown(SDL_SCANCODE_S)) camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (inputController.IsKeyDown(SDL_SCANCODE_D)) camera.ProcessKeyboard(RIGHT, deltaTime);

            // Mouse movement
            float xpos = inputController.GetMouseX();
            float ypos = inputController.GetMouseY();

            if (firstMouse) {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // reversed

            lastX = xpos;
            lastY = ypos;

            camera.ProcessMouseMovement(xoffset, yoffset);
        }

         //Game Loop
        // Automatically update all objects in the scene
        // for (const auto& obj : sceneRoots) {
        //     obj->Update(deltaTime);
        // }

        if (m_isPlaying) {
            for (const auto& root : sceneRoots) {
                    root->Update(deltaTime); // This call is recursive by design
                }
            // --- STEP 2: GATHER COLLIDERS ---
            // Create a flat list of all colliders in the scene for easy checking.
            std::vector<ColliderComponent*> allColliders;
            for (const auto& root : sceneRoots) {
                GetAllColliders(root, allColliders);
            }


            if (!allColliders.empty()){
                // This will print every frame, but it's useful for a quick test
                std::cout << "Collision check loop running with " << allColliders.size() << " colliders." << std::endl; 
            }
            // --- STEP 3: COLLISION CHECK AND RESPONSE ---
            // Check every collider against every other collider.
            // The j = i + 1 is an optimization to avoid checking the same pair twice.
            for (size_t i = 0; i < allColliders.size(); ++i) {
                for (size_t j = i + 1; j < allColliders.size(); ++j) {
                    ColliderComponent* a = allColliders[i];
                    ColliderComponent* b = allColliders[j];

                            // --- NEW ENHANCED DEBUG PRINT ---
                printf("Checking: '%s' vs '%s'\n", a->m_owner->name.c_str(), b->m_owner->name.c_str());
                printf("  '%s' Min(%.2f, %.2f, %.2f) Max(%.2f, %.2f, %.2f)\n", 
                    a->m_owner->name.c_str(), a->worldMin.x, a->worldMin.y, a->worldMin.z, a->worldMax.x, a->worldMax.y, a->worldMax.z);
                printf("  '%s' Min(%.2f, %.2f, %.2f) Max(%.2f, %.2f, %.2f)\n", 
                    b->m_owner->name.c_str(), b->worldMin.x, b->worldMin.y, b->worldMin.z, b->worldMax.x, b->worldMax.y, b->worldMax.z);
                // --- END OF NEW DEBUG PRINT ---

                    // Don't check for collisions between two static objects
                    bool aIsStatic = a->m_owner->physics ? a->m_owner->physics->isStatic : true;
                    bool bIsStatic = b->m_owner->physics ? b->m_owner->physics->isStatic : true;
                    if (aIsStatic && bIsStatic) {
                        continue;
                    }

                    // Perform the AABB collision check
                    if (ColliderComponent::CheckCollision(*a, *b)) {
                        // A collision has occurred!
                        std::cout << "!!!!!!!! COLLISION DETECTED between " 
                                << a->m_owner->name << " and " << b->m_owner->name 
                                << " !!!!!!!!" << std::endl;

                        // --- COLLISION RESPONSE LOGIC ---
                        // Identify which object is dynamic and which is static
                        ColliderComponent* dynamicCollider = aIsStatic ? b : a;
                        ColliderComponent* staticCollider = aIsStatic ? a : b;
                        
                        // For this simple case, we assume one is static and one is dynamic.
                        // A more advanced engine would handle dynamic-dynamic collisions.

                        // Calculate how much the dynamic object has penetrated the static one on the Y-axis
                        float penetrationY = staticCollider->worldMax.y - dynamicCollider->worldMin.y;
                        
                        // Move the dynamic object straight up by the penetration amount to resolve the collision
                        dynamicCollider->m_owner->position.y += penetrationY;

                        // Stop the object's downward velocity so it doesn't try to fall through again next frame
                        if (dynamicCollider->m_owner->physics->velocity.y < 0) {
                            dynamicCollider->m_owner->physics->velocity.y = 0;
                        }

                        // IMPORTANT: After moving the object, we must update its collider's world bounds immediately
                        dynamicCollider->Update();
                    }
                }
            }
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        //imgui window
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Settings");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("fov");               // Display some text (you can use a format strings too)
            // ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            // ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("Roation angle ", &rotation_angle, 1.0f, 5.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            // ImGui::SliderFloat("Camera FOV", &camera.FOV, 45.0f, 120.0f);
            
            ImGui::SliderFloat("Light intensity ", &lightIntensity, 0.0f, 2.0f);
            ImGui::SliderFloat("Emissive intensity", &emission_strength, 0.0f, 10.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            //get color from imgu and set as lightColor as glm::vec3
            ImGui::ColorEdit3("Point light color", (float*)&pointlightColor);
            ImGui::ColorEdit3("Spot light color", (float*)&spotlightColor);
            //vec3 input for light position
            // ImGui::InputFloat3("light position", (float*)&lightPos);

            // if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //     counter++;
            // ImGui::SameLine();
            // ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }


            // Render Scene Controls (Play/Stop button)
        ImGui::Begin("Scene Controls");
        if (m_isPlaying) {
            if (ImGui::Button("Stop")) {
                StopSimulation();
            }
        } else {
            if (ImGui::Button("Play")) {
                StartSimulation();
            }
        }
        ImGui::End();
        // --- MAIN MENU BAR ---
if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Project...")) {
            // Use tinyfiledialogs to ask the user for a new, empty folder
            const char* folderPath = tinyfd_selectFolderDialog("Select a New Empty Folder for Project", "");
            if (folderPath) {
                if (projectManager.CreateNewProject(folderPath)) {
                    // Success! Clear the old scene.
                    selectedObject.reset();
                    sceneRoots.clear();
                    isSceneDirty = false; 
                } else {
                    tinyfd_messageBox("Error", "Could not create project. The folder must be empty.", "ok", "error", 1);
                }
            }
        }

        if (ImGui::MenuItem("Load Project...")) {
            // Ask for the .myproj file
            const char* supportedFiles[1] = { "*.myproj" };
            const char* filePath = tinyfd_openFileDialog("Load Project", "", 1, supportedFiles, "MyEngine Project", 0);
            if (filePath) {
                if (projectManager.LoadProject(filePath)) {
                    // Project is loaded, now load its scene
                    selectedObject.reset();
                    projectManager.LoadScene(sceneRoots);
                    isSceneDirty = false; 
                } else {
                    tinyfd_messageBox("Error", "Could not load the selected project file.", "ok", "error", 1);
                }
            }
        }
        
        // The "Save" item should only be clickable if a project is loaded
        if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, projectManager.IsProjectLoaded())) {
            if (projectManager.SaveScene(sceneRoots)) {
                // --- THIS IS THE FIX ---
                // Replace the tinyfd call with your own ImGui-based notification
                AddNotification("Scene saved successfully!");
                // tinyfd_notifyPopup("Saved", "Scene saved successfully!", "info"); // DELETE THIS
            } else {
                AddNotification("Error: Failed to save scene!");
            }
        }

        ImGui::Separator();
        
        // In the "File" menu
        if (ImGui::MenuItem("Exit")) {
            if (isSceneDirty) {
                // If there are unsaved changes, show the popup.
                showExitPopup = true;
            } else {
                // If the scene is clean, exit immediately.
                is_running = false;
            }
        }
        ImGui::EndMenu();
    }
    
    // --- SCENE MENU ---
    // This menu will handle adding objects to the scene
    if (ImGui::BeginMenu("Scene", projectManager.IsProjectLoaded())) {
        if (ImGui::MenuItem("Import Model...")) {
            const char* supportedFiles[2] = { "*.obj", "*.fbx" }; // Add more as needed
            const char* filePath = tinyfd_openFileDialog("Import Model", "", 2, supportedFiles, "3D Models", 0);
            if (filePath) {
                // This is the full asset pipeline in action!
                std::string relativeCachePath = projectManager.ProcessAndImportAsset(filePath);
                if (!relativeCachePath.empty()) {
                    // Asset was processed, now load it into the scene
                    auto resourceManager = projectManager.GetResourceManager(); // You'll need to add this getter
                    auto model = resourceManager->LoadModel(relativeCachePath);
                    auto newObject = std::make_shared<GameObject>(model, std::filesystem::path(filePath).stem().string());
                    sceneRoots.push_back(newObject);
                }
            }
        }
        
        ImGui::Separator();
        
        if (ImGui::BeginMenu("Add Primitive")) {
            if (ImGui::MenuItem("Cube")) {
                auto resourceManager = projectManager.GetResourceManager();
                auto model = resourceManager->LoadModel("primitive::cube");
                auto newObject = std::make_shared<GameObject>(model, "Cube");
                sceneRoots.push_back(newObject);
            }
            if (ImGui::MenuItem("Plane")) {
                 auto resourceManager = projectManager.GetResourceManager();
                auto model = resourceManager->LoadModel("primitive::plane");
                auto newObject = std::make_shared<GameObject>(model, "Plane");
                sceneRoots.push_back(newObject);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Add Light")) {
             if (ImGui::MenuItem("Directional Light")) {
                auto light = std::make_shared<DirectionalLight>();
                sceneRoots.push_back(light);
                lightManager.RegisterLight(light.get()); // Don't forget to register it
            }
            if (ImGui::MenuItem("Point Light")) {
                auto light = std::make_shared<PointLight>();
                sceneRoots.push_back(light);
                lightManager.RegisterLight(light.get());
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    
    ImGui::EndMainMenuBar();
}
        
        // --- IMGUI UI CODE in your main loop ---

        static std::weak_ptr<GameObject> selectedObject; // Store selected object

        // --- 1. Scene Hierarchy Window ---
        ImGui::Begin("Scene Hierarchy");

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            // ...but NOT on any actual item. If so, we clear the selection.
            if (!ImGui::IsAnyItemHovered()) {
                selectedObject.reset(); // .reset() clears a weak_ptr
            }
        }


        // Recursive function to draw the hierarchy tree
        std::function<void(const std::shared_ptr<GameObject>&)> drawNode =
            [&](const std::shared_ptr<GameObject>& node) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

            // Highlight the node if it's the currently selected one
            if (selectedObject.lock() == node) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            // Mark as a leaf if it has no children
            if (node->GetChildren().empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            // Draw the actual tree node
            bool node_open = ImGui::TreeNodeEx(node.get(), flags, "%s", node->name.c_str());

            // --- FIX #2: More Robust Selection Logic ---
            // Check if THIS SPECIFIC item was clicked with the left mouse button
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                selectedObject = node;
            }


            // --- START OF DELETE FIX ---
            // This creates a context menu that opens on right-click for THIS tree node.
            if (ImGui::BeginPopupContextItem()) {
                ImGui::Text("Actions for %s", node->name.c_str());
                ImGui::Separator();
                if (ImGui::MenuItem("Delete")) {
                    // Don't delete immediately! Just mark it for deletion.
                    objectToDelete = node; 
                }
                // You could add "Duplicate", "Add Child", etc. here in the future
                ImGui::EndPopup();
            }
            // If the node is open, recurse for all children
            if (node_open) {
                for (const auto& child : node->GetChildren()) {
                    drawNode(child); // Recurse
                }
                ImGui::TreePop();
            }
        };

        // Draw all root objects
        for (const auto& root : sceneRoots) { // your list of root objects
            drawNode(root);
        }

        ImGui::End();

        if (objectToDelete) {
            // If the object to delete was the selected one, unselect it.
            if (selectedObject.lock() == objectToDelete) {
                selectedObject.reset();
            }
            
            // This is a complex operation. We need a function to handle it.
            // Let's assume you create a helper function like this:
            RemoveObjectFromScene(objectToDelete, sceneRoots);

            // Clear the pointer so we don't delete it again next frame.
            objectToDelete = nullptr;
            // You should set your "scene dirty" flag here.
        }
        // --- 2. Inspector Window ---
        ImGui::Begin("Inspector");

        if (auto obj = selectedObject.lock()) {
            // --- START OF RENAMING FIX ---
            
            // Create a buffer to hold the name for editing.
            // ImGui's InputText works with C-style char arrays.
            char nameBuffer[128];
            // Copy the object's name into the buffer. strncpy is safer than strcpy.
            strncpy(nameBuffer, obj->name.c_str(), sizeof(nameBuffer));
            // Ensure null termination, just in case the name was too long.
            nameBuffer[sizeof(nameBuffer) - 1] = 0;

            // Create the input text field.
            ImGui::Text("Name");
            ImGui::SameLine();
            if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                // This block executes when the user presses Enter.
                obj->name = std::string(nameBuffer);
                //set dirty flag 
                isSceneDirty = true;
            }
            // Also update on losing focus (clicking away)
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                obj->name = std::string(nameBuffer);
                isSceneDirty = true;
            }
            // "##Name" makes the label invisible, since we already have a "Name" text widget.
            
            // --- END OF RENAMING FIX ---
            
            ImGui::Separator();
            obj->OnImGui(isSceneDirty);
        } else {
            ImGui::Text("No object selected.");
        }

        ImGui::End();

            if (showExitPopup) {
            ImGui::OpenPopup("Unsaved Changes");
        }

        // Always center this window when it appears
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Unsaved Changes", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("You have unsaved changes. Are you sure you want to exit?\nAll unsaved work will be lost.\n\n");
            ImGui::Separator();

            if (ImGui::Button("Save and Exit", ImVec2(120, 0))) {
                if (projectManager.SaveScene(sceneRoots)) {
                    is_running = false; // Exit after successful save
                } else {
                    // Handle save error if necessary
                    AddNotification("Error: Could not save scene!");
                }
                ImGui::CloseCurrentPopup();
                showExitPopup = false;
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            if (ImGui::Button("Exit Without Saving", ImVec2(150, 0))) {
                is_running = false; // User chose to lose work
                ImGui::CloseCurrentPopup();
                showExitPopup = false;
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                showExitPopup = false; // User cancelled, just close the popup
            }
            ImGui::EndPopup();
        }

        // Rendering
        RenderNotifications();
        ImGui::Render();

        

        // glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render depth of scene from light view to texture
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        lightProjection =  glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(dirLight->position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        //set face culling to front
        glCullFace(GL_FRONT);

        //render scene from light view
        depthShader->use();
        depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);



        // ShadowRenderScene(*depthShader, sceneRoots, lightManager);
        RenderSceneGeometryOnly(*depthShader, sceneRoots);
        // 2. SWITCH BACK TO DEFAULT FRAMEBUFFER
        // ====================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //reset viewport
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //set face culling to normal for normal rendering 
        glCullFace(GL_BACK);

        // render Depth map to quad for visual debugging
        // ---------------------------------------------
        // debugQuadShader->use();
        // debugQuadShader->setFloat("near_plane", near_plane);
        // debugQuadShader->setFloat("far_plane", far_plane);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, depthMap);
        // renderQuad();


        //Activate Shaders
        glm::mat4 projection  = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view  = glm::mat4(1.0f);
        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        mainShader->use();
        //set global texureunit to 0 for color pass
        //Global textureUnit counter
        int textureUnit = 0;
        

        mainShader->setMat4("projection", projection);
        mainShader->setMat4("view", view);
        mainShader->setVec3("viewPos", camera.Position);
        mainShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        lightManager.UploadLightsToShader(*mainShader);

        // Bind the depth map texture to a specific texture unit (e.g., unit 0)
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        mainShader->setInt("shadowMap", textureUnit); // Tell the main shader the shadow map is in unit 0
        textureUnit++;

        // B. Render the scene using the configured shader
        RenderSceneWithMaterial(*mainShader, sceneRoots, textureUnit);
        // RenderScene(*mainShader, view, projection, camera.Position, sceneRoots, lightManager);
        
        // //render light cube
        //Lighting
        // lightCubeShader->use();
        // mainShader->use();
        
        // lightCubeShader->setMat4("projection", projection);
        // lightCubeShader->setMat4("view", view);
        // lightCubeShader->setVec3("lightColor", pointlightColor);
        // mainShader->use();
        // lightManager.UploadLightsToShader(*mainShader);


        // Set other global uniforms like view position
        // mainShader->setVec3("viewPos", camera.Position);
        //  mainShader->setMat4("projection", projection);
        // // mainShader->setMat4("view", view);
        // mainShader->setVec3("viewPos", camera.Position); 

        // mainShader->setVec3("lightColor", lightColor);
        // mainShader->setFloat("lightIntensity",  lightIntensity);


        //render loop
        // Automatically draw all objects in the scene
        // for (const auto& obj : sceneRoots) {
        //     obj->Draw(*mainShader, view, projection);
        // }

        // we now draw as many light bulbs as we have point lights.
        // glBindVertexArray(lightCubeVAO);
        // for (unsigned int i = 0; i < 4; i++)
        // {
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, pointLightPositions[i]);
        // model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        // lightCubeShader->setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // }

        //render cubes
        
        // mainShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        // glm::vec3 diffuseColor = lightColor   * lightIntensity; //glm::vec3(0.5f); // decrease the influence
        // glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
        // mainShader.setVec3("light.ambient", ambientColor);
        // mainShader.setVec3("light.diffuse", diffuseColor);
        // mainShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        // mainShader.setVec3("lightColor", lightColor * );
        // mainShader.setFloat("lightIntensity", lightIntensity);
        // mainShader.setVec3("viewPos", camera.Position);


        // material properties
        // mainShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        // mainShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        // mainShader.setVec3("material.specular", 1.0f, 1.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
        // mainShader->setFloat("material.shininess", 100.0f);
        // mainShader.setFloat("emission_strength", emission_strength);
        // mainShader.setFloat("time",float(SDL_GetTicks() / 10000.f) );

        // directional light
        // mainShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        // mainShader->setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
        // mainShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        // mainShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // // point light 1
        // mainShader->setVec3("pointLights[0].position", pointLightPositions[0]);
        // mainShader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        // mainShader->setVec3("pointLights[0].diffuse", pointlightColor * lightIntensity);
        // mainShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        // mainShader->setFloat("pointLights[0].constant", 1.0f);
        // mainShader->setFloat("pointLights[0].linear", 0.09f);
        // mainShader->setFloat("pointLights[0].quadratic", 0.032f);
        // // point light 2
        // mainShader->setVec3("pointLights[1].position", pointLightPositions[1]);
        // mainShader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        // mainShader->setVec3("pointLights[1].diffuse", pointlightColor * lightIntensity);
        // mainShader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        // mainShader->setFloat("pointLights[1].constant", 1.0f);
        // mainShader->setFloat("pointLights[1].linear", 0.09f);
        // mainShader->setFloat("pointLights[1].quadratic", 0.032f);
        // // point light 3
        // mainShader->setVec3("pointLights[2].position", pointLightPositions[2]);
        // mainShader->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        // mainShader->setVec3("pointLights[2].diffuse", pointlightColor * lightIntensity);
        // mainShader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        // mainShader->setFloat("pointLights[2].constant", 1.0f);
        // mainShader->setFloat("pointLights[2].linear", 0.09f);
        // mainShader->setFloat("pointLights[2].quadratic", 0.032f);
        // // point light 4
        // mainShader->setVec3("pointLights[3].position", pointLightPositions[3]);
        // mainShader->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        // mainShader->setVec3("pointLights[3].diffuse", pointlightColor * lightIntensity);
        // mainShader->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        // mainShader->setFloat("pointLights[3].constant", 1.0f);
        // mainShader->setFloat("pointLights[3].linear", 0.09f);
        // mainShader->setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight
        // mainShader->setVec3("spotLight.position", camera.Position);
        // mainShader->setVec3("spotLight.direction", camera.Front);
        // mainShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        // mainShader->setVec3("spotLight.diffuse", spotlightColor * lightIntensity);
        // mainShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        // mainShader->setFloat("spotLight.constant", 1.0f);
        // mainShader->setFloat("spotLight.linear", 0.09f);
        // mainShader->setFloat("spotLight.quadratic", 0.032f);
        // mainShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        // mainShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        
        // mainShader->setMat4("projection", projection);
        // mainShader->setMat4("view", view);
        // mainShader->setVec3("viewPos", camera.Position); 

        // world transformation
        // model = glm::mat4(1.0f);
        // mainShader.setMat4("model", model);

        // mainShader.setVec3("lightPos", lightPos);

        //render obj model
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        // model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
        // // model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        // // model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        // mainShader->setMat4("model", model);

        // suzan.Draw(mainShader);
        // Backpack.Draw(mainShader);
        // Sponza.Draw(mainShader);

        // mainShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);

        // bind diffuse map
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // // bind specular map
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, specularMap);
        // // bind emission map
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, emissionMap);


        

        // camera/view transformation
        // glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        // float radius = 10.0f;
        // float camX = static_cast<float>(sin((float)SDL_GetTicks() / 1000.0f) * radius);
        // float camZ = static_cast<float>(cos((float)SDL_GetTicks() / 1000.0f) * radius);
        // view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // mainShader.setMat4("view", view);
        // // camera/view transformation
        // glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        // mainShader.setMat4("view", view);


        
        
        // unsigned int transformLoc = glGetUniformLocation(mainShader.ID, "transform");
        // glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

        //update our uniform color
        // float timeValue = (float)SDL_GetTicks() / 1000.0f;
        // float redValue = sin(timeValue) / 2.0f + 0.5f;
        // float greenValue = cos(timeValue) / 2.0f + 0.5f;
        // float blueValue = sin(timeValue) / 2.0f + 0.5f;
        // int vertexColorLocation = glGetUniformLocation(mainShader.ID, "ourColor");
        // glUniform4f(vertexColorLocation, redValue, greenValue, blueValue, 1.0f);
        
        // //setup to use our texture
        // glUniform1i(glGetUniformLocation(mainShader.ID, "texture1"), 0);
        // // mainShader.setInt("texture2", 0);
        // glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0,4);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time 

        // // render boxes
        // glBindVertexArray(VAO);
        // for (unsigned int i = 0; i < 10; i++)
        // {
        //     // calculate the model matrix for each object and pass it to shader before drawing
        //     model = glm::mat4(1.0f);
        //     model = glm::translate(model, cubePositions[i]);
        //     float angle;
        //     if(i==0){
        //         angle = 20.0f * rotation_angle;
        //     }else{
        //         angle = 20.0f * (i * rotation_angle);
        //     }
        //     // std::cout<< angle << "\n" ;
        //     model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        //     mainShader->setMat4("model", model);

        //     glDrawArrays(GL_TRIANGLES, 0, 36);

        //     // std::cout << "angle: " << angle << std::endl;
            
        // }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    

    }

    //de allocate all resouces
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
    // glDeleteProgram(lightCubeShader->ID);

    glDeleteProgram(mainShader->ID);
    glDeleteProgram(debugQuadShader->ID);
    glDeleteProgram(depthShader->ID);

    // Assimp::DefaultLogger::kill();
    
    //at end clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

