#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <glad/glad.h>
#include <cmath>
#include "shader.h"
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
    // float vertices[] = {
    //     -0.5f, -0.5f, 0.0f, // left  
    //     0.5f, -0.5f, 0.0f, // right 
    //     0.0f,  0.5f, 0.0f  // top
    // };

    // //rectangle
    // float vertices[] = {
    // 0.5f,  0.5f, 0.0f,  // top right
    // 0.5f, -0.5f, 0.0f,  // bottom right
    // -0.5f, -0.5f, 0.0f,  // bottom left
    // -0.5f,  0.5f, 0.0f   // top left 
    // };
    // unsigned int indices[] = {  // note that we start from 0!
    //     0, 1, 3,   // first triangle
    //     1, 2, 3    // second triangle
    // }; 

    //my custom shape
    float vertices[] = {
        0.0f, 0.4f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.2f, 0.2f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.2f, 0.2f, 0.0f, 0.0f, 0.0f, 0.1f,
        -0.2f, -0.2f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.2f, -0.2f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, -0.4f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.4f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.4f, 0.0f, 0.0f , 1.0f, 0.0f, 0.0f

    };

    unsigned int indices[] = {
        0,1,2,
        1,3,2,
        4,3,1,
        4,5,3,
        2,3,6,
        7,4,1
    };

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
    
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    //set offset and stride to get correct color and position data
    //postion data first 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //color data second
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    

    


    //to enable wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bool is_running = true;
    
    SDL_Event e;
    while(is_running){
        while (SDL_PollEvent(&e)){
            if(e.type == SDL_EVENT_QUIT) is_running = false;
        }
        
        //clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        
        //drawing my first triangle
        //glUseProgram(ourShader);
        ourShader.use();
        //update our uniform color
        float timeValue = (float)SDL_GetTicks() / 1000.0f;
        float redValue = sin(timeValue) / 2.0f + 0.5f;
        float greenValue = cos(timeValue) / 2.0f + 0.5f;
        float blueValue = sin(timeValue) / 2.0f + 0.5f;
        int vertexColorLocation = glGetUniformLocation(ourShader.ID, "ourColor");
        glUniform4f(vertexColorLocation, redValue, greenValue, blueValue, 1.0f);

        glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0,4);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
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
