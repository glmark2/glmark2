#include "oglsdl.h"

#include "screen.h"
#include "scene.h"

int main(int argc, char *argv[])
{
    SDL_Event event;
    int running = 1;
    unsigned current_scene = 0;
    Screen screen;

    printf("===================================================\n");
    printf("    GLMark 08\n");
    printf("===================================================\n");
    if(!screen.init())
        return 0;
    printf("===================================================\n");

    // Create the scenes.
    Scene *scene[] = {
        new SceneBuild(screen),
        new SceneTexture(screen),
        new SceneShading(screen),
    };

    unsigned num_scenes = sizeof(scene) / sizeof(*scene);

    // Load the first scene
    if (!scene[current_scene]->load())
        return 1;
    scene[current_scene]->start();

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

        // Draw the next state of the current scene
        scene[current_scene]->update();
        scene[current_scene]->draw();

        // If the scene has finished, move to the next one
        if (!scene[current_scene]->is_running()) {
            // Unload this scene
            scene[current_scene]->unload();

            current_scene++;

            // Do we have another scene?
            if (current_scene < num_scenes) {
                // Load and start next scene
                if (!scene[current_scene]->load())
                    return 1;
                scene[current_scene]->start();
            }
            else
                running = false;
        }


        screen.update();
    }

    unsigned score = 0;
    for (unsigned i = 0; i < num_scenes; i++)
        score += scene[i]->calculate_score();

    printf("===================================================\n");
    printf("Your GLMark08 Score is %u  ^_^\n", score);
    printf("===================================================\n");

    return 0;
}
