/*
 * Copyright © 2008 Ben Smith
 * Copyright © 2010-2011 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
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
 *  Marc Ordinas i Llopis, Collabora Ltd. (pulsar scene)
 *  Jesse Barker (glmark2)
 */
#ifndef GLMARK2_SCENE_H_
#define GLMARK2_SCENE_H_

#include "gl-headers.h"

#include "mesh.h"
#include "vec.h"
#include "program.h"

#include <math.h>

#include <string>
#include <map>
#include <list>
#include <vector>
#include "canvas.h"

/**
 * A configurable scene used for creating benchmarks.
 */
class Scene
{
public:
    virtual ~Scene();

    /**
     * Scene options.
     */
    struct Option {
        Option(const std::string &nam, const std::string &val, const std::string &desc,
               const std::string &values = "");

        Option() {}
        std::string name;
        std::string value;
        std::string default_value;
        std::string description;
        std::vector<std::string> acceptable_values;
        bool set;
    };

    struct Stats {
        double average_frame_time;
        double average_user_time;
        double average_system_time;
        double cpu_busy_percent;
        double shader_compilation_time;
    };

    /**
     * The result of a validation check.
     */
    enum ValidationResult {
        ValidationFailure,
        ValidationSuccess,
        ValidationUnknown
    };

    /**
     * Checks whether this scene (in its current configuration) is supported.
     *
     * @param show_errors whether to log errors about unsupported features
     *
     * @return whether the scene is supported
     */
    virtual bool supported(bool show_errors);

    /**
     * Updates the scene state.
     */
    virtual void update();

    /**
     * Draws the current scene state.
     */
    virtual void draw();

    /**
     * Gets an informational string describing the scene.
     *
     * @param title if specified, a custom title to use, instead of the default
     */
    virtual std::string info_string(const std::string &title = "");

    /**
     * Sets the value of an option for this scene.
     *
     * @param opt the option to set
     * @param val the value to set the option to
     *
     * @return whether the option value was set successfully
     */
    virtual bool set_option(const std::string &opt, const std::string &val);

    /**
     * Validates the current output of this scene.
     *
     * This method should be called after having called ::draw() once.
     *
     * @return the validation result
     */
    virtual ValidationResult validate() { return ValidationUnknown; }

    /**
     * Gets whether this scene is running.
     *
     * @return true if running, false otherwise
     */
    bool running() { return running_; }

    /**
     * Sets whether this scene is running.
     *
     * @return true if running, false otherwise
     */
    void running(bool r) { running_ = r; }

    /**
     * Gets the average FPS value for this scene.
     *
     * @return the average FPS value
     */
    unsigned average_fps();

    /**
     * Gets the statistics for this scene.
     *
     * @return the statistics
     */
    Stats stats();

    /**
     * Gets the name of the scene.
     * @return the name of the scene
     */
    const std::string &name() { return name_; }

    /**
     * Resets all scene options to their default values.
     */
    void reset_options();

    /**
     * Sets the default value of a scene option.
     */
    bool set_option_default(const std::string &opt, const std::string &val);

    /**
     * Gets the scene options.
     *
     * @return the scene options
     */
    const std::map<std::string, Option> &options() { return options_; }

    /**
     * Prepare for a benchmarking run.
     */
    bool prepare();

    /**
     * Finish a benchmarking run.
     */
    void finish();

    /**
     * Gets a dummy scene object reference.
     *
     * @return the dummy Scene
     */
    static Scene &dummy()
    {
        static Scene dummy_scene(Canvas::dummy(), "");
        return dummy_scene;
    }

    /**
     * Loads a shader program from a pair of vertex and fragment shader strings.
     *
     * @return whether the operation succeeded
     */
    static bool load_shaders_from_strings(Program &program,
                                          const std::string &vtx_shader,
                                          const std::string &frg_shader,
                                          const std::string &vtx_shader_filename = "None",
                                          const std::string &frg_shader_filename = "None");

protected:
    Scene(Canvas &pCanvas, const std::string &name);
    std::string construct_title(const std::string &title);

    /**
     * Performs option-independent resource loading and configuration.
     *
     * It should be safe to call ::load() (and the corresponding ::unload())
     * only once per program execution, although you may choose to do so more
     * times to better manage resource consumption.
     *
     * @return whether loading succeeded
     */
    virtual bool load();

    /**
     * Performs option-independent resource unloading.
     */
    virtual void unload();

    /**
     * Performs option-dependent resource loading and configuration.
     *
     * This method also prepares a scene for a benchmark run.
     * It should be called just before running a scene/benchmark.
     *
     * The base Scene::setup() method also checks whether a scene
     * configuration is supported by calling ::supported(true).
     *
     * @return whether setting the scene up succeeded
     */
    virtual bool setup();

    /**
     * Performs option-dependent resource unloading.
     *
     * This method should be called just after running a scene/benchmark.
     *
     * @return the operation status
     */
    virtual void teardown();

    /**
     * Updates the elapsed times for this benchmark run.
     */
    void update_elapsed_times();

    struct ElapsedTime {
        double start = 0.0;
        double lastUpdate = 0.0;
        double elapsed() { return lastUpdate - start; }
    };

    static double shaderCompilationTime_;
    Canvas &canvas_;
    std::string name_;
    std::map<std::string, Option> options_;
    ElapsedTime realTime_;
    ElapsedTime userTime_;
    ElapsedTime systemTime_;
    ElapsedTime idleTime_;
    unsigned currentFrame_;
    bool running_;
    double duration_;      // Duration of run in seconds
    unsigned nframes_;
};

/*
 * Special Scene used for setting the default options
 */
class SceneDefaultOptions : public Scene
{
public:
    SceneDefaultOptions(Canvas &pCanvas) : Scene(pCanvas, "") {}
    bool set_option(const std::string &opt, const std::string &val);

protected:
    bool setup();

private:
    std::list<std::pair<std::string, std::string> > defaultOptions_;
};

class SceneBuild : public Scene
{
public:
    SceneBuild(Canvas &pCanvas);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneBuild();

protected:
    bool load();
    void unload();
    bool setup();
    void teardown();

    Program program_;
    LibMatrix::mat4 perspective_;
    LibMatrix::vec3 centerVec_;
    float radius_;
    Mesh mesh_;
    bool orientModel_;
    float orientationAngle_;
    LibMatrix::vec3 orientationVec_;
    float rotation_;
    float rotationSpeed_;
    bool useVbo_;
};

class SceneTexture : public Scene
{
public:
    SceneTexture(Canvas &pCanvas);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneTexture();

protected:
    bool load();
    void unload();
    bool setup();
    void teardown();

    Program program_;
    Mesh mesh_;
    GLuint texture_;
    float radius_;
    bool orientModel_;
    float orientationAngle_;
    LibMatrix::vec3 orientationVec_;
    LibMatrix::mat4 perspective_;
    LibMatrix::vec3 centerVec_;
    LibMatrix::vec3 rotation_;
    LibMatrix::vec3 rotationSpeed_;
};

class SceneShading : public Scene
{
public:
    SceneShading(Canvas &pCanvas);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneShading();

protected:
    bool load();
    void unload();
    bool setup();
    void teardown();

    Program program_;
    float radius_;
    bool orientModel_;
    float orientationAngle_;
    LibMatrix::vec3 orientationVec_;
    LibMatrix::vec3 centerVec_;
    LibMatrix::mat4 perspective_;
    Mesh mesh_;
    float rotation_;
    float rotationSpeed_;
};

class SceneGrid : public Scene
{
public:
    SceneGrid(Canvas &pCanvas, const std::string &name);
    void update();
    void draw();
    virtual ValidationResult validate();

    ~SceneGrid();

protected:
    bool load();
    void unload();
    bool setup();
    void teardown();

    Program program_;
    Mesh mesh_;
    float rotation_;
    float rotationSpeed_;
};

class SceneConditionals : public SceneGrid
{
public:
    SceneConditionals(Canvas &pCanvas);
    ValidationResult validate();

    ~SceneConditionals();

protected:
    bool setup();
};

class SceneFunction : public SceneGrid
{
public:
    SceneFunction(Canvas &pCanvas);
    ValidationResult validate();

    ~SceneFunction();

protected:
    bool setup();
};

class SceneLoop : public SceneGrid
{
public:
    SceneLoop(Canvas &pCanvas);
    ValidationResult validate();

    ~SceneLoop();

protected:
    bool setup();
};

class SceneBump : public Scene
{
public:
    SceneBump(Canvas &pCanvas);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneBump();

protected:
    bool load();
    void unload();
    bool setup();
    void teardown();

    Program program_;
    Mesh mesh_;
    GLuint texture_;
    float rotation_;
    float rotationSpeed_;
private:
    bool setup_model_plain(const std::string &type);
    bool setup_model_normals();
    bool setup_model_normals_tangent();
    bool setup_model_height();
};

class SceneEffect2D : public Scene
{
public:
    SceneEffect2D(Canvas &pCanvas);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneEffect2D();

protected:
    bool load();
    void unload();
    bool setup();
    void teardown();

    Program program_;
    Mesh mesh_;
    GLuint texture_;
};

class ScenePulsar : public Scene
{
public:
    ScenePulsar(Canvas &pCanvas);
    void update();
    void draw();
    ValidationResult validate();

    ~ScenePulsar();

protected:
    bool load();
    void unload();
    bool setup();
    void teardown();

    int numQuads_;
    Program program_;
    Mesh mesh_;
    LibMatrix::vec3 scale_;
    std::vector<LibMatrix::vec3> rotations_;
    std::vector<LibMatrix::vec3> rotationSpeeds_;
    GLuint texture_;

private:
    void create_and_setup_mesh();
};

struct SceneDesktopPrivate;

class SceneDesktop : public Scene
{
public:
    SceneDesktop(Canvas &canvas);
    bool supported(bool show_errors);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneDesktop();

protected:
    bool setup();
    void teardown();

private:
    SceneDesktopPrivate *priv_;
};

struct SceneBufferPrivate;

class SceneBuffer : public Scene
{
public:
    SceneBuffer(Canvas &canvas);
    bool supported(bool show_errors);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneBuffer();

private:
    bool setup();
    void teardown();
    SceneBufferPrivate *priv_;
};

class SceneIdeasPrivate;

class SceneIdeas : public Scene
{
public:
    SceneIdeas(Canvas &pCanvas);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneIdeas();

protected:
    bool setup();
    void teardown();

private:
    SceneIdeasPrivate* priv_;
};

class SceneTerrainPrivate;

class SceneTerrain : public Scene
{
public:
    SceneTerrain(Canvas &pCanvas);
    bool supported(bool show_errors);
    void update();
    void draw();
    ValidationResult validate();

    ~SceneTerrain();

protected:
    bool setup();
    void teardown();

private:
    SceneTerrainPrivate* priv_;
};

class JellyfishPrivate;
class SceneJellyfish : public Scene
{
    JellyfishPrivate* priv_;
public:
    SceneJellyfish(Canvas &pCanvas);
    ~SceneJellyfish();
    void update();
    void draw();
    ValidationResult validate();

protected:
    bool setup();
    void teardown();
};

class ShadowPrivate;
class SceneShadow : public Scene
{
    ShadowPrivate* priv_;
public:
    SceneShadow(Canvas& canvas);
    bool supported(bool show_errors);
    void update();
    void draw();
    ValidationResult validate();

protected:
    bool setup();
    void teardown();
};

class RefractPrivate;
class SceneRefract : public Scene
{
    RefractPrivate* priv_;
public:
    SceneRefract(Canvas& canvas);
    bool supported(bool show_errors);
    void update();
    void draw();
    ValidationResult validate();

protected:
    bool setup();
    void teardown();
};

class SceneClear : public Scene
{
public:
    SceneClear(Canvas &pCanvas);
};

#endif
