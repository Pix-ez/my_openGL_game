#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <glad/glad.h>
#include <cmath>
#include <string>
#include <format>
#include "shader.h"
#include "camera.h"
#include "inputController.h"
// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
#include "utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

//settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

InputController inputController;

int main(int argc, char *argv[]){

    // --- GET THE BASE PATH TO THE EXECUTABLE ---
    const char* basePath_c = SDL_GetBasePath();
    if (!basePath_c) {
        std::cerr << "Error getting base path: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::string basePath(basePath_c);
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
    glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f); 

    
    bool enable_mouse = false;  // false = gameplay mode
    bool last_mouse_enabled = true; // force initial mode setup
    bool escapePressed = false;
    bool was_mouse_enabled_last_frame = true;

    // // camera
    // glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    // glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    // glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    // timing
    // float deltaTime = 0.0f;	// time between current frame and last frame
    // float lastFrame = 0.0f;

    glEnable(GL_DEPTH_TEST);
    


  
    //user our custom shader
    std::string vertPath = basePath + "shader/shader.vert";
    std::string fragPath = basePath + "shader/shader.frag";
    Shader lightingShader(vertPath.c_str(), fragPath.c_str());


    std::string lightCube_vert = basePath + "shader/light_cube.vert";
    std::string lightCube_frag = basePath + "shader/light_cube.frag";
    Shader lightCubeShader(lightCube_vert.c_str(), lightCube_frag.c_str());
    
    // Shader lightingShader("src/shader.vert", "src/shader.frag");
    //vertex data and buffers
    // The OG traingle 
    // float vertices[] = {
    //     -0.5f, -0.5f, 0.0f, // left  
    //     0.5f, -0.5f, 0.0f, // right 
    //     0.0f,  0.5f, 0.0f  // top
    // };

    // //Texture co-ordinates
    // float texCoords[] = {
    //     0.0f,0.0f, //left bottom
    //     1.0f, 0.0f, //right bottom
    //     0.5f, 1.0f //center top
    // };
    
   
    //rectangle
    // float vertices[] = {
    //     // positions          // colors           // texture coords
    //     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
    //     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    //     -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    //     -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    // };
    // unsigned int indices[] = {  // note that we start from 0!
    //     0, 1, 3,   // first triangle
    //     1, 2, 3    // second triangle
    // }; 

    //my custom shape
    // float vertices[] = {
    //     0.0f, 0.4f, 0.0f, 1.0f, 0.0f, 0.0f,
    //     0.2f, 0.2f, 0.0f, 0.0f, 1.0f, 0.0f,
    //     -0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.1f,
    //     -0.2f, -0.2f, 0.0f, 1.0f, 0.0f, 0.0f,
    //     0.2f, -0.2f, 0.0f, 0.0f, 1.0f, 0.0f,
    //     0.0f, -0.4f, 0.0f, 1.0f, 0.0f, 0.0f,
    //     -0.4f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    //     0.4f, 0.0f, 0.0f , 1.0f, 0.0f, 0.0f
    
    // };

    // unsigned int indices[] = {
    //     0,1,2,
    //     1,3,2,
    //     4,3,1,
    //     4,5,3,
    //     2,3,6,
    //     7,4,1
    // };

    // unsigned int EBO ,VBO, VAO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    // glGenBuffers(1,&EBO);

    // // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    // glBindVertexArray(VAO);
    
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    //  // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // // color attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);
    // // texture coord attribute
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray(2);
  
       // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        //vertex pos          texture coords vertex normals 
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,    0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,    0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,    0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,    0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,   0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f,
    };
    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    // positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // //generate texture
    // unsigned int texture1, texture2;
    // //texture1
    // glGenTextures(1, &texture1);    
    // glBindTexture(GL_TEXTURE_2D, texture1);
    // //setup texture setting
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // //setup texture filtering
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // //load image
    // int width, height, nChannels;
    // stbi_set_flip_vertically_on_load(true);
    // // unsigned char* data = stbi_load("textures/brick_wall_diffuse.jpg", &width, &height, &nChannels, 0);
    // std::string texturePath = basePath + "textures/brick_wall_diffuse.jpg";
    // unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nChannels, 0);
    // if(data){
    //     printf("loaded texture %dx%d with %d channels\n", width, height, nChannels);
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // }else{
    //     std::cerr << "Failed to load texture" << std::endl;
    // }
    // //free image data
    // stbi_image_free(data);

    // //texture2
    // glGenTextures(1, &texture2);    
    // glBindTexture(GL_TEXTURE_2D, texture2);
    // //setup texture setting
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // //setup texture filtering
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // //load image
    // stbi_set_flip_vertically_on_load(true);
    // // data = stbi_load("textures/cat.png", &width, &height, &nChannels, 0);
    // texturePath = basePath + "textures/cat.png";
    // data = stbi_load(texturePath.c_str(), &width, &height, &nChannels, 0);
    // if(data){
    //     printf("loaded texture %dx%d with %d channels\n", width, height, nChannels);
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // }else{
    //     std::cerr << "Failed to load texture" << std::endl;
    // }
    // //free image data
    // stbi_image_free(data);
    //set offset and stride to get correct color and position data
    // //postion data first 
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // //color data second
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    // glEnableVertexAttribArray(1);
    
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);
    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture((basePath + "textures/container2.png").c_str());
    unsigned int specularMap = loadTexture((basePath + "textures/container2_specular.png").c_str());
    unsigned int emissionMap = loadTexture((basePath + "textures/matrix.jpg").c_str());
    

    
     // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
     // -------------------------------------------------------------------------------------------
    lightingShader.use(); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:
    // glUniform1i(glGetUniformLocation(lightingShader.ID, "texture1"), 0);
    // or set it via the texture class
    // lightingShader.setInt("texture1", 0);
    // lightingShader.setInt("texture2", 1);
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setInt("material.emission", 2);
 


    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

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

            inputController.BeginFrame();

            inputController.ProcessEvent(event);

            if(event.type == SDL_EVENT_QUIT) is_running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) is_running = false;

            //Camera control events
            //for keyboard press w key 
            // if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_W)
            // {
            //     // cameraPos += cameraSpeed * cameraFront;
            //     camera.ProcessKeyboard(FORWARD, deltaTime);
            //     std::cout << "w key pressed" << std::endl;
            // }
            // //for keyboard press s key 
            // if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_S)
            // {
            //     // cameraPos -= cameraSpeed * cameraFront;
            //     camera.ProcessKeyboard(BACKWARD, deltaTime);
            // }
            // //for keyboard press a key 
            // if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_A)
            // {
            //     // cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            //     camera.ProcessKeyboard(LEFT, deltaTime);
            // }
            // //for keyboard press d key 
            // if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_D)
            // {
            //     // cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            //     camera.ProcessKeyboard(RIGHT, deltaTime);
            // }


            if (event.type == SDL_EVENT_KEY_DOWN && event.key.repeat == 0) {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    escapePressed = true;
                }
            }




        ImGui_ImplSDL3_ProcessEvent(&event); // Forward your event to backend
        }


        
        //enable/disable mouse cursor on pressing escape key
        // if(inputController.WasKeyPressed(SDL_SCANCODE_ESCAPE)){
        //     enable_mouse = !enable_mouse;
        //     // std::cout << "escape key pressed" << std::endl;
        // } 

        //disable mouse cursor on pressing left mouse button
        // if(inputController.WasMousePressed(0)) {
        //     enable_mouse = false;
        //     // std::cout << "left mouse button pressed" << std::endl;
        // }

        if (escapePressed) {
            enable_mouse = !enable_mouse;
            std::cout << "ESC pressed. enable_mouse = " << enable_mouse << "\n";
        }
        

        //disable mouse cursor
        // if(!enable_mouse){
        //     SDL_HideCursor();
        //     SDL_SetWindowRelativeMouseMode(window, true);

        //     //if mouse disable capture keyboard input
        //     (void)SDL_GetRelativeMouseState(nullptr, nullptr); //to avoid abrupt mouse and camera movement

        //     if(inputController.IsKeyDown(SDL_SCANCODE_W)) camera.ProcessKeyboard(FORWARD, deltaTime);
        //     if(inputController.IsKeyDown(SDL_SCANCODE_A)) camera.ProcessKeyboard(LEFT, deltaTime);
        //     if(inputController.IsKeyDown(SDL_SCANCODE_S)) camera.ProcessKeyboard(BACKWARD, deltaTime);
        //     if(inputController.IsKeyDown(SDL_SCANCODE_D)) camera.ProcessKeyboard(RIGHT, deltaTime);

        //     float xpos = inputController.GetMouseX();
        //     float ypos = inputController.GetMouseY();

        //     if(firstMouse) {
        //         lastX = xpos;
        //         lastY = ypos;
        //         firstMouse = false;
        //     }

        //     float xoffset = xpos - lastX;
        //     float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        //     lastX = xpos;
        //     lastY = ypos;

        //     camera.ProcessMouseMovement(xoffset, yoffset);
        // }else{
        //     SDL_ShowCursor();
        //     SDL_SetWindowRelativeMouseMode(window, false);
        //     firstMouse = true;
        // }
        
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
            ImGui::InputFloat3("light position", (float*)&lightPos);

            // if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //     counter++;
            // ImGui::SameLine();
            // ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

           
        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        


        //bind texture to corresponding texture unit
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture1);

        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, texture2);
        

        // //glm test
        // glm::mat4 trans = glm::mat4(1.0f); //initalize with indentity matrix
        // // trans =  glm::rotate(trans , glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
        // trans =  glm::rotate(trans , glm::radians(rotation_angle_z), glm::vec3(0.0, 0.0, 1.0));
        // // trans =  glm::rotate(trans , 8*(float(SDL_GetTicks()) / 1000.0f), glm::vec3(0.0, 1.0, 0.0));
        // trans = glm::scale(trans, glm::vec3(1.0, 1.0, 1.0));


        //Activate Shader
        
        glm::mat4 projection  = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view  = glm::mat4(1.0f);
        
        // //render light cube
        //Lighting
        lightCubeShader.use();

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        lightCubeShader.setVec3("lightColor", pointlightColor);

        // world transformation
        // model = glm::translate(model, lightPos);
        // model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        // lightCubeShader.setMat4("model", model);

        // render the cube
        glBindVertexArray(lightCubeVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 36);

         // we now draw as many light bulbs as we have point lights.
         glBindVertexArray(lightCubeVAO);
         for (unsigned int i = 0; i < 4; i++)
         {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
         }

        //render cubes
        lightingShader.use();

        // lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        // glm::vec3 diffuseColor = lightColor   * lightIntensity; //glm::vec3(0.5f); // decrease the influence
        // glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
        // lightingShader.setVec3("light.ambient", ambientColor);
        // lightingShader.setVec3("light.diffuse", diffuseColor);
        // lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        // lightingShader.setVec3("lightColor", lightColor * );
        // lightingShader.setVec3("light.position", lightPos);
        lightingShader.setFloat("lightIntensity", lightIntensity);
        // lightingShader.setVec3("viewPos", camera.Position);


        // material properties
        // lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        // lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        // lightingShader.setVec3("material.specular", 1.0f, 1.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
        lightingShader.setFloat("material.shininess", 100.0f);
        lightingShader.setFloat("emission_strength", emission_strength);
        lightingShader.setFloat("time",float(SDL_GetTicks() / 10000.f) );

        // directional light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[0].diffuse", pointlightColor * lightIntensity);
        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09f);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[1].diffuse", pointlightColor * lightIntensity);
        lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.09f);
        lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[2].diffuse", pointlightColor * lightIntensity);
        lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[2].constant", 1.0f);
        lightingShader.setFloat("pointLights[2].linear", 0.09f);
        lightingShader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[3].diffuse", pointlightColor * lightIntensity);
        lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[3].constant", 1.0f);
        lightingShader.setFloat("pointLights[3].linear", 0.09f);
        lightingShader.setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", spotlightColor * lightIntensity);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        //create transformations
        // glm::mat4 model = glm::mat4(1.0f); //make sure intinal matrix is identity
        // model = glm::rotate(model, glm::radians(rotation_angle_z), glm::vec3(1.0f, 0.0f, 0.0f));
        // view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); //pulling camera -3 in z 

        // std::cout << camera.getFOV() << std::endl;
        // projection = glm::perspective(glm::radians(camera.FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        
        // lightingShader.setMat4("view", view);
        // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        lightingShader.setMat4("projection", projection);

        // camera/view transformation
        view = camera.GetViewMatrix();
        lightingShader.setMat4("view", view);
        // lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        // bind emission map
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, emissionMap);


        

        // lightingShader.setVec3("lightColor", lightColor);
        // lightingShader.setFloat("lightIntensity",  lightIntensity);

        // camera/view transformation
        // glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        // float radius = 10.0f;
        // float camX = static_cast<float>(sin((float)SDL_GetTicks() / 1000.0f) * radius);
        // float camZ = static_cast<float>(cos((float)SDL_GetTicks() / 1000.0f) * radius);
        // view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // lightingShader.setMat4("view", view);
        // camera/view transformation
        // glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        // lightingShader.setMat4("view", view);


        
        
        // unsigned int transformLoc = glGetUniformLocation(lightingShader.ID, "transform");
        // glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

        //update our uniform color
        // float timeValue = (float)SDL_GetTicks() / 1000.0f;
        // float redValue = sin(timeValue) / 2.0f + 0.5f;
        // float greenValue = cos(timeValue) / 2.0f + 0.5f;
        // float blueValue = sin(timeValue) / 2.0f + 0.5f;
        // int vertexColorLocation = glGetUniformLocation(lightingShader.ID, "ourColor");
        // glUniform4f(vertexColorLocation, redValue, greenValue, blueValue, 1.0f);
        
        // //setup to use our texture
        // glUniform1i(glGetUniformLocation(lightingShader.ID, "texture1"), 0);
        // // lightingShader.setInt("texture2", 0);
        // glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0,4);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time 

        // render boxes
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle;
            if(i==0){
                angle = 20.0f * rotation_angle;
            }else{
                angle = 20.0f * (i * rotation_angle);
            }
            // std::cout<< angle << "\n" ;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);

            // std::cout << "angle: " << angle << std::endl;
            
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    

    }

    //de allocate all resouces
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
    glDeleteProgram(lightingShader.ID);

    //at end clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

