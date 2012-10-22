//
// Copyright © 2012 Linaro Limited
//
// This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
//
// glmark2 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// glmark2.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors:
//  Jesse Barker
//
#include "scene.h"
#include "model.h"
#include "util.h"
#include "log.h"
#include "shader-source.h"
#include "stack.h"

using std::string;
using std::vector;
using std::map;
using LibMatrix::Stack4;
using LibMatrix::mat4;
using LibMatrix::vec4;
using LibMatrix::vec3;

static const vec4 lightPosition(0.0f, 3.0f, 2.0f, 1.0f);

//
// To create a shadow map, we need a framebuffer object set up for a 
// depth-only pass.  The render target can then be bound as a texture,
// and the depth values sampled from that texture can be used in the
// distance-from-light computations when rendering the shadow on the
// ground below the rendered object.
//
class DepthRenderTarget
{
    Program program_;
    unsigned int canvas_width_;
    unsigned int canvas_height_;
    unsigned int width_;
    unsigned int height_;
    unsigned int tex_;
    unsigned int fbo_;
public:
    DepthRenderTarget() :
        canvas_width_(0),
        canvas_height_(0),
        width_(0),
        height_(0),
        tex_(0),
        fbo_(0) {}
    ~DepthRenderTarget() {}
    bool setup(unsigned int width, unsigned int height);
    void teardown();
    void enable(const mat4& mvp);
    void disable();
    unsigned int depthTexture() { return tex_; }
    Program& program() { return program_; }
};

bool
DepthRenderTarget::setup(unsigned int width, unsigned int height)
{
    canvas_width_ = width;
    canvas_height_ = height;
    width_ = canvas_width_ * 2;
    height_ = canvas_height_ * 2;

    static const string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/depth.vert");
    static const string frg_shader_filename(GLMARK_DATA_PATH"/shaders/depth.frag");

    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    if (!Scene::load_shaders_from_strings(program_, vtx_source.str(), frg_source.str())) {
        return false;
    }

    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width_, height_, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           tex_, 0);
    unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Log::error("DepthRenderState::setup: glCheckFramebufferStatus failed (0x%x)\n", status);
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

void
DepthRenderTarget::teardown()
{
    program_.stop();
    program_.release();
    if (tex_) {
        glDeleteTextures(1, &tex_);
        tex_ = 0;
    }
    if (fbo_) {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }
}

void
DepthRenderTarget::enable(const mat4& mvp)
{
    program_.start();
    program_["ModelViewProjectionMatrix"] = mvp;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           tex_, 0);
    glViewport(0, 0, width_, height_);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void DepthRenderTarget::disable()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, canvas_width_, canvas_height_);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

class ShadowPrivate
{
    Canvas& canvas_;
    DepthRenderTarget depthTarget_;
    Program program_;
    Stack4 modelview_;
    Stack4 projection_;
    Mesh mesh_;
    vec3 centerVec_;
    float radius_;
    float rotation_;
    float rotationSpeed_;
    bool useVbo_;
    
public:
    ShadowPrivate(Canvas& canvas) :
        canvas_(canvas),
        radius_(0.0),
        rotation_(0.0),
        rotationSpeed_(36.0),
        useVbo_(true) {}
    ~ShadowPrivate() {}

    bool setup(map<string, Scene::Option>& options);
    void teardown();
    void update(double elapsedTime);
    void draw();
};

bool
ShadowPrivate::setup(map<string, Scene::Option>& options)
{
    // Program object setup
    static const string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic.vert");
    static const string frg_shader_filename(GLMARK_DATA_PATH"/shaders/light-basic.frag");
    static const vec4 materialDiffuse(1.0f, 1.0f, 1.0f, 1.0f);

    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    vtx_source.add_const("LightSourcePosition", lightPosition);
    vtx_source.add_const("MaterialDiffuse", materialDiffuse);

    if (!Scene::load_shaders_from_strings(program_, vtx_source.str(), frg_source.str())) {
        return false;
    }

    // Model setup
    Model::find_models();
    Model model;
    bool modelLoaded = model.load("horse");

    if(!modelLoaded) {
        return false;
    }

    model.calculate_normals();

    // Mesh setup
    vector<std::pair<Model::AttribType, int> > attribs;
    attribs.push_back(std::pair<Model::AttribType, int>(Model::AttribTypePosition, 3));
    attribs.push_back(std::pair<Model::AttribType, int>(Model::AttribTypeNormal, 3));
    model.convert_to_mesh(mesh_, attribs);

    useVbo_ = (options["use-vbo"].value == "true");
    bool interleave = (options["interleave"].value == "true");
    mesh_.vbo_update_method(Mesh::VBOUpdateMethodMap);
    mesh_.interleave(interleave);

    if (useVbo_) {
        mesh_.build_vbo();
    }
    else {
        mesh_.build_array();
    }

    // Calculate a projection matrix that is a good fit for the model
    vec3 maxVec = model.maxVec();
    vec3 minVec = model.minVec();
    vec3 diffVec = maxVec - minVec;
    centerVec_ = maxVec + minVec;
    centerVec_ /= 2.0;
    float diameter = diffVec.length();
    radius_ = diameter / 2;
    float fovy = 2.0 * atanf(radius_ / (2.0 + radius_));
    fovy /= M_PI;
    fovy *= 180.0;
    float aspect(static_cast<float>(canvas_.width())/static_cast<float>(canvas_.height()));
    projection_.perspective(fovy, aspect, 2.0, 2.0 + diameter);

    if (!depthTarget_.setup(canvas_.width(), canvas_.height())) {
        Log::error("Failed to set up the render target for the depth pass\n");
        return false;
    }

    return true;
}


void
ShadowPrivate::teardown()
{
    depthTarget_.teardown();
    program_.stop();
    program_.release();
    mesh_.reset();
}

void
ShadowPrivate::update(double elapsedTime)
{
    rotation_ = rotationSpeed_ * elapsedTime;
}

void
ShadowPrivate::draw()
{
    // To perform the depth pass, set up the model-view transformation so
    // that we're looking at the horse from the light position.  That will
    // give us the appropriate view for the shadow.
    modelview_.push();
    modelview_.loadIdentity();
    modelview_.lookAt(lightPosition.x(), lightPosition.y(), lightPosition.z(),
                      0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0);
    modelview_.rotate(rotation_, 0.0f, 1.0f, 0.0f);
    mat4 mvp(projection_.getCurrent());
    mvp *= modelview_.getCurrent();
    modelview_.pop();

    // Enable the depth render target with our transformation and render.
    depthTarget_.enable(mvp);
    vector<GLint> attrib_locations;
    attrib_locations.push_back(depthTarget_.program()["position"].location());
    attrib_locations.push_back(depthTarget_.program()["normal"].location());
    mesh_.set_attrib_locations(attrib_locations);
    if (useVbo_) {
        mesh_.render_vbo();
    }
    else {
        mesh_.render_array();
    }
    depthTarget_.disable();

    // TODO: Ground rendering using the above generated texture...

    // Draw the "normal" view of the horse
    modelview_.push();
    modelview_.translate(-centerVec_.x(), -centerVec_.y(), -(centerVec_.z() + 2.0 + radius_));
    modelview_.rotate(rotation_, 0.0f, 1.0f, 0.0f);
    mvp = projection_.getCurrent();
    mvp *= modelview_.getCurrent();

    program_.start();
    program_["ModelViewProjectionMatrix"] = mvp;

    // Load the NormalMatrix uniform in the shader. The NormalMatrix is the
    // inverse transpose of the model view matrix.
    LibMatrix::mat4 normal_matrix(modelview_.getCurrent());
    normal_matrix.inverse().transpose();
    program_["NormalMatrix"] = normal_matrix;
    attrib_locations.clear();
    attrib_locations.push_back(program_["position"].location());
    attrib_locations.push_back(program_["normal"].location());
    mesh_.set_attrib_locations(attrib_locations);
    if (useVbo_) {
        mesh_.render_vbo();
    }
    else {
        mesh_.render_array();
    }

    // Per-frame cleanup
    modelview_.pop();
}

SceneShadow::SceneShadow(Canvas& canvas) :
    Scene(canvas, "shadow"),
    priv_(0)
{
    options_["use-vbo"] = Scene::Option("use-vbo", "true",
                                        "Whether to use VBOs for rendering",
                                        "false,true");
    options_["interleave"] = Scene::Option("interleave", "false",
                                           "Whether to interleave vertex attribute data",
                                           "false,true");
}

SceneShadow::~SceneShadow()
{
    delete priv_;
}

bool
SceneShadow::supported(bool show_errors)
{
    static const string oes_depth_texture("GL_OES_depth_texture");
    static const string arb_depth_texture("GL_ARB_depth_texture");
    if (!GLExtensions::support(oes_depth_texture) &&
        !GLExtensions::support(arb_depth_texture)) {
        if (show_errors) {
            Log::error("We do not have the depth texture extension!!!\n");
        }

        return false;
    }
    return true;
}

bool
SceneShadow::load()
{
    running_ = false;
    return true;
}

void
SceneShadow::unload()
{
}

bool
SceneShadow::setup()
{
    // If the scene isn't supported, don't bother to go through setup.
    if (!supported(false) || !Scene::setup())
    {
        return false;
    }

    priv_ = new ShadowPrivate(canvas_);
    if (!priv_->setup(options_)) {
        delete priv_;
        priv_ = 0;
        return false;
    }

    // Set core scene timing after actual initialization so we don't measure
    // set up time.
    startTime_ = Util::get_timestamp_us() / 1000000.0;
    lastUpdateTime_ = startTime_;
    running_ = true;

    return true;
}

void
SceneShadow::teardown()
{
    // Add scene-specific teardown here
    priv_->teardown();
    Scene::teardown();
}

void
SceneShadow::update()
{
    Scene::update();
    // Add scene-specific update here
    priv_->update(lastUpdateTime_ - startTime_);
}

void
SceneShadow::draw()
{
    priv_->draw();
}

Scene::ValidationResult
SceneShadow::validate()
{
    return Scene::ValidationUnknown;
}
