#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <glad/glad.h>
#include <cmath>
#include <string>
#include <format>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY ;

int main(int argc, char *argv[]){
    if(!SDL_Init(SDL_INIT_VIDEO)){
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("My OpenGl APP",
                                        SCR_WIDTH, SCR_HEIGHT, windowFlags);

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


    //compile our shaders
    //vertex shader
    // unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // glCompileShader(vertexShader);
    // //check for shader compile errors
    // int sucess;
    // char infolog[512];
    // glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &sucess);
    // if(!sucess){
    //     glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
    //     std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
    // }

    // //fragment shader
    // unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // glCompileShader(fragmentShader);
    // //check error
    // glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &sucess);
    // if(!sucess){
    //     glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
    //     std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << std::endl;
    // }

    // //link shaders
    // unsigned int shaderProgram = glCreateProgram();
    // glAttachShader(shaderProgram, vertexShader);
    // glAttachShader(shaderProgram, fragmentShader);
    // glLinkProgram(shaderProgram);
    // //check error
    // glGetProgramiv(shaderProgram, GL_LINK_STATUS, &sucess);
    // if(!sucess){
    //     glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
    //     std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
    // }
    // //clearn up shader
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);

    //user our custom shader
    
    Shader ourShader("src/shader.vert", "src/shader.frag");
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
    float vertices[] = {
        // positions          // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 

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

    unsigned int EBO ,VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1,&EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
     // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    //generate texture
    unsigned int texture1, texture2;
    //texture1
    glGenTextures(1, &texture1);    
    glBindTexture(GL_TEXTURE_2D, texture1);
    //setup texture setting
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //setup texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("textures/brick_wall_diffuse.jpg", &width, &height, &nChannels, 0);
    if(data){
        printf("loaded texture %dx%d with %d channels\n", width, height, nChannels);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        std::cerr << "Failed to load texture" << std::endl;
    }
    //free image data
    stbi_image_free(data);

    //texture2
    glGenTextures(1, &texture2);    
    glBindTexture(GL_TEXTURE_2D, texture2);
    //setup texture setting
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //setup texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("textures/cat.png", &width, &height, &nChannels, 0);
    if(data){
        printf("loaded texture %dx%d with %d channels\n", width, height, nChannels);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        std::cerr << "Failed to load texture" << std::endl;
    }
    //free image data
    stbi_image_free(data);
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
    

    
     // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
     // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:
    // glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    // or set it via the texture class
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);


    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    //to enable wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bool is_running = true;
    
    SDL_Event e;
    while(is_running){
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        Uint64 deltaCounter = currentCounter - prevCounter;
        prevCounter = currentCounter;

        double deltaTime = double(deltaCounter) / SDL_GetPerformanceFrequency();
        accumulatedTime += deltaTime;
        fpsCounter++;

        // Update FPS display every fpsUpdateInterval seconds
        if (accumulatedTime >= fpsUpdateInterval) {
            double fps = fpsCounter / accumulatedTime;
            std::string title = std::format("{} - FPS: {:.1f}", baseTitle, fps);
            SDL_SetWindowTitle(window, title.c_str());

            accumulatedTime = 0.0;
            fpsCounter = 0;
        }
        while (SDL_PollEvent(&e)){
            if(e.type == SDL_EVENT_QUIT) is_running = false;
        }
        
        //clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        //bind texture to corresponding texture unit
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        
        //drawing my first triangle
        //glUseProgram(ourShader);

         //glm test
        glm::mat4 trans = glm::mat4(1.0f); //initalize with indentity matrix
        // trans =  glm::rotate(trans , glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
        trans =  glm::rotate(trans , float(SDL_GetTicks()) / 1000.0f, glm::vec3(0.0, 0.0, 1.0));
        trans =  glm::rotate(trans , 8*(float(SDL_GetTicks()) / 1000.0f), glm::vec3(0.0, 1.0, 0.0));
        trans = glm::scale(trans, glm::vec3(1.0, 1.0, 1.0));

        
        
        ourShader.use();
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
        //update our uniform color
        // float timeValue = (float)SDL_GetTicks() / 1000.0f;
        // float redValue = sin(timeValue) / 2.0f + 0.5f;
        // float greenValue = cos(timeValue) / 2.0f + 0.5f;
        // float blueValue = sin(timeValue) / 2.0f + 0.5f;
        // int vertexColorLocation = glGetUniformLocation(ourShader.ID, "ourColor");
        // glUniform4f(vertexColorLocation, redValue, greenValue, blueValue, 1.0f);
        
        // //setup to use our texture
        // glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
        // // ourShader.setInt("texture2", 0);
        glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0,4);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time 

        
        SDL_GL_SwapWindow(window);
    

    }

    //de allocate all resouces
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(ourShader.ID);

    //at end clean up
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
