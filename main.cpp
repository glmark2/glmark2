#include "oglsdl.h"

#include "screen.h"
#include "scene.h"

int main(int argc, char *argv[])
{
    SDL_Event event;
    int running = 1;
    unsigned current_scene = 0;
    Screen screen;
   
    SceneBuild scenebuild;
    SceneTexture scenetexture;
    SceneShading sceneshading;
 
    printf("===================================================\n");
    printf("    GLMark 08\n");
    printf("===================================================\n");
    
    printf("===================================================\n");
    if(!screen.init())
        return 0;
    printf("===================================================\n");

    if(!scenebuild.load() || !scenetexture.load() || !sceneshading.load())
        return 0;

    while(running)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE)
                    running = 0;
                break;
            }
        }

        screen.clear();

        switch(current_scene)
        {
        case 0:
            current_scene++;
            scenebuild.start();
//            scenebuild.mRunning = false;
            break;
        case 1:
            scenebuild.update();
            scenebuild.draw();
            if(!scenebuild.mRunning)
            {
                current_scene++;
                scenetexture.start();
//                scenetexture.mRunning = false;
            }
            break;
        case 2:
            scenetexture.update();
            scenetexture.draw();
            if(!scenetexture.mRunning)
            {
                current_scene++;
                sceneshading.start();
//                sceneshading.mRunning = false;
            }
            break;
        case 3:
            sceneshading.update();
            sceneshading.draw();
            if(!sceneshading.mRunning)
                running = false;
            break;
        }
        
        screen.update();
    }
    
    scenebuild.calculate_score();
    scenetexture.calculate_score();
    sceneshading.calculate_score();
    
    unsigned score = scenebuild.mScore + scenetexture.mScore + sceneshading.mScore;

    printf("===================================================\n");
    printf("Your GLMark08 Score is %u  ^_^\n", score);
    printf("===================================================\n");

    return 0;
}
