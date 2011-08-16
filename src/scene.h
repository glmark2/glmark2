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
            name(nam), value(val), default_value(val), description(desc) {}
        Option() {}
        std::string name;
        std::string value;
        std::string default_value;
        std::string description;
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

    const std::string &name() { return mName; }
    virtual bool set_option(const std::string &opt, const std::string &val);
    void reset_options();
    bool set_option_default(const std::string &opt, const std::string &val);
    const std::map<std::string, Option> &options() { return mOptions; }

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
    double pixel_value_distance(Canvas::Pixel p1, Canvas::Pixel p2,
                                bool use_alpha=false);

    Canvas &mCanvas;
    std::string mName;
    std::map<std::string, Option> mOptions;

    double mStartTime;
    double mLastUpdateTime;
    unsigned mCurrentFrame;
    unsigned mAverageFPS;      // Average FPS of run

    bool mRunning;
    double mDuration;      // Duration of run in seconds
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
    std::list<std::pair<std::string, std::string> > mDefaultOptions;
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
    Program mProgram;

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
    bool mUseVbo;
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
    Program mProgram;

    Mesh mCubeMesh;
    GLuint mTexture;
    LibMatrix::vec3 mRotation;
    LibMatrix::vec3 mRotationSpeed;
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
    Program mProgram;

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
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
    Program mProgram;

    Mesh mMesh;
    float mRotation;
    float mRotationSpeed;
};

class SceneConditionals : public SceneGrid
{
public:
    SceneConditionals(Canvas &pCanvas);
    void setup();

    ~SceneConditionals();
};

class SceneFunction : public SceneGrid
{
public:
    SceneFunction(Canvas &pCanvas);
    void setup();

    ~SceneFunction();
};

class SceneLoop : public SceneGrid
{
public:
    SceneLoop(Canvas &pCanvas);
    void setup();

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
    Program mProgram;

    Mesh mMesh;
    GLuint mTexture;
    float mRotation;
    float mRotationSpeed;
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
#endif
