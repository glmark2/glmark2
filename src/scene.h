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
 */
#ifndef GLMARK2_SCENE_H_
#define GLMARK2_SCENE_H_

#include "gl-headers.h"

#include "mesh.h"
#include "model.h"
#include "texture.h"
#include "vec.h"
#include "program.h"

#include <math.h>

#include <string>
#include <map>
#include <list>

class Scene
{
public:
    virtual ~Scene();

    struct Option {
        Option(const std::string &nam, const std::string &val, const std::string &desc) :
            name(nam), value(val), default_value(val), description(desc), set(false) {}
        Option() {}
        std::string name;
        std::string value;
        std::string default_value;
        std::string description;
        bool set;
    };

    enum ValidationResult {
        ValidationFailure,
        ValidationSuccess,
        ValidationUnknown
    };

    // load() and unload() handle option-independent configuration.
    // It should be safe to call these only once per program execution,
    // although you may choose to do so more times to better manage
    // resource consumption.
    virtual int load();
    virtual void unload();

    // setup() and teardown() handle option-dependent configuration and
    // also prepare a scene for a benchmark run.
    // They should be called just before and after running a scene/benchmark.
    virtual void setup();
    virtual void teardown();

    virtual void update();
    virtual void draw();
    virtual std::string info_string(const std::string &title = "");

    unsigned average_fps();
    bool is_running();

    const std::string &name() { return name_; }
    virtual bool set_option(const std::string &opt, const std::string &val);
    void reset_options();
    bool set_option_default(const std::string &opt, const std::string &val);
    const std::map<std::string, Option> &options() { return options_; }

    static Scene &dummy()
    {
        static Scene dummy_scene(Canvas::dummy(), "");
        return dummy_scene;
    }

    virtual ValidationResult validate() { return ValidationUnknown; }

    static bool load_shaders_from_strings(Program &program,
                                          const std::string &vtx_shader,
                                          const std::string &frg_shader,
                                          const std::string &vtx_shader_filename = "None",
                                          const std::string &frg_shader_filename = "None");
    static bool load_shaders_from_files(Program &program,
                                        const std::string &vtx_shader_filename,
                                        const std::string &frg_shader_filename);

    static uint64_t get_timestamp_us();

protected:
    Scene(Canvas &pCanvas, const std::string &name);
    std::string construct_title(const std::string &title);

    Canvas &canvas_;
    std::string name_;
    std::map<std::string, Option> options_;
    double startTime_;
    double lastUpdateTime_;
    unsigned currentFrame_;
    unsigned averageFPS_;      // Average FPS of run
    bool running_;
    double duration_;      // Duration of run in seconds
};

/*
 * Special Scene used for setting the default options
 */
class SceneDefaultOptions : public Scene
{
public:
    SceneDefaultOptions(Canvas &pCanvas) : Scene(pCanvas, "") {}
    bool set_option(const std::string &opt, const std::string &val);
    void setup();

private:
    std::list<std::pair<std::string, std::string> > defaultOptions_;
};

class SceneBuild : public Scene
{
public:
    SceneBuild(Canvas &pCanvas);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneBuild();

protected:
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
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneTexture();

protected:
    Program program_;
    Mesh mesh_;
    GLuint texture_;
    LibMatrix::vec3 rotation_;
    LibMatrix::vec3 rotationSpeed_;
};

class SceneShading : public Scene
{
public:
    SceneShading(Canvas &pCanvas);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneShading();

protected:
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
    virtual int load();
    virtual void unload();
    virtual void setup();
    virtual void teardown();
    virtual void update();
    virtual void draw();
    virtual ValidationResult validate();

    ~SceneGrid();

protected:
    Program program_;
    Mesh mesh_;
    float rotation_;
    float rotationSpeed_;
};

class SceneConditionals : public SceneGrid
{
public:
    SceneConditionals(Canvas &pCanvas);
    void setup();
    ValidationResult validate();

    ~SceneConditionals();
};

class SceneFunction : public SceneGrid
{
public:
    SceneFunction(Canvas &pCanvas);
    void setup();
    ValidationResult validate();

    ~SceneFunction();
};

class SceneLoop : public SceneGrid
{
public:
    SceneLoop(Canvas &pCanvas);
    void setup();
    ValidationResult validate();

    ~SceneLoop();
};

class SceneBump : public Scene
{
public:
    SceneBump(Canvas &pCanvas);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneBump();

protected:
    Program program_;
    Mesh mesh_;
    GLuint texture_;
    float rotation_;
    float rotationSpeed_;
private:
    void setup_model_plain(const std::string &type);
    void setup_model_normals();
};

class SceneEffect2D : public Scene
{
public:
    SceneEffect2D(Canvas &pCanvas);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneEffect2D();

protected:
    Program program_;

    Mesh mesh_;
    GLuint texture_;
};

class ScenePulsar : public Scene
{
public:
    ScenePulsar(Canvas &pCanvas);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~ScenePulsar();

protected:
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
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneDesktop();

private:
    SceneDesktopPrivate *priv_;
};

struct SceneBufferPrivate;

class SceneBuffer : public Scene
{
public:
    SceneBuffer(Canvas &canvas);
    int load();
    void unload();
    void setup();
    void teardown();
    void update();
    void draw();
    ValidationResult validate();

    ~SceneBuffer();

private:
    SceneBufferPrivate *priv_;
};
#endif
