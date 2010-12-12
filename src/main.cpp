/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010 Linaro
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Ben Smith (original glmark benchmark)
 *  Alexandros Frantzis (glmark2)
 */
#include "oglsdl.h"
#include "scene.h"

#if USE_GL
#include "screen-sdl-gl.h"
#elif USE_GLESv2
#include "screen-sdl-glesv2.h"
#endif

#define UNUSED_PARAM(x) (void)(x)

int main(int argc, char *argv[])
{
    UNUSED_PARAM(argc);
    UNUSED_PARAM(argv);

    SDL_Event event;
    int running = 1;
    unsigned current_scene = 0;

    // Create the screen
#if USE_GL
    ScreenSDLGL screen(800, 600, 24, 0);
#elif USE_GLESv2
    ScreenSDLGLESv2 screen(800, 600, 24, 0);
#endif

    if (!screen.mInitSuccess) {
        printf("Error: %s: Could not initialize screen\n", __FUNCTION__);
        return 1;
    }

    printf("===================================================\n");
    printf("    glmark2 %s\n", GLMARK_VERSION);
    printf("===================================================\n");
    screen.print_info();
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

        // Update the state of the current scene
        scene[current_scene]->update();

        // If the current scene is still running then draw it,
        // otherwise move to the next scene
        if (scene[current_scene]->is_running()) {
            scene[current_scene]->draw();
        }
        else {
            // Unload the current scene
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
    printf("Your glmark2 Score is %u  ^_^\n", score);
    printf("===================================================\n");

    return 0;
}
