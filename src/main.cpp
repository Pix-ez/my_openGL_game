#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <GL/gl.h>



int main(int argc, char *argv[]){
    if(!SDL_Init(SDL_INIT_VIDEO)){
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("My OpenGl APP",
                                        800,600, SDL_WINDOW_OPENGL);

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

    bool is_running = true;
    
    SDL_Event e;
    while(is_running){
        while (SDL_PollEvent(&e)){
            if(e.type == SDL_EVENT_QUIT) is_running = false;
        }
        SDL_GL_SwapWindow(window);

    }

    //at end clean up
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}