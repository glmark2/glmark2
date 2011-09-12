/*
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
 *  Alexandros Frantzis (glmark2)
 */
#include <cmath>

#include "scene.h"
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"
#include "program.h"
#include "shader-source.h"
#include "util.h"

enum BlurDirection {
    BlurDirectionHorizontal,
    BlurDirectionVertical,
    BlurDirectionBoth
};

static void
create_blur_shaders(ShaderSource& vtx_source, ShaderSource& frg_source,
                    unsigned int radius, float sigma, BlurDirection direction)
{
    vtx_source.append_file(GLMARK_DATA_PATH"/shaders/desktop.vert");
    frg_source.append_file(GLMARK_DATA_PATH"/shaders/desktop-blur.frag");

    /* Don't let the gaussian curve become too narrow */
    sigma = sigma >= 1.0 ? sigma : 1.0;

    unsigned int side = 2 * radius + 1;

    for (size_t i = 0; i < radius + 1; i++) {
        float s2 = 2.0 * sigma * sigma;
        float k = 1.0 / std::sqrt(M_PI * s2) * std::exp( - ((float)i * i) / s2);
        std::stringstream ss_tmp;
        ss_tmp << "Kernel" << i;
        frg_source.add_const(ss_tmp.str(), k);
    }

    std::stringstream ss;
    ss << "result = " << std::endl;
   
    if (direction == BlurDirectionHorizontal) {
        for (size_t i = 0; i < side; i++) {
            int offset = (int)(i - radius);
            ss << "texture2D(Texture0, TextureCoord + vec2(" <<
                  offset << ".0 * TextureStepX, 0.0)) * Kernel" <<
                  std::abs(offset) << " +" << std::endl;
        }
        ss << "0.0 ;" << std::endl;
    }
    else if (direction == BlurDirectionVertical) {
        for (size_t i = 0; i < side; i++) {
            int offset = (int)(i - radius);
            ss << "texture2D(Texture0, TextureCoord + vec2(0.0, " <<
                  offset << ".0 * TextureStepY)) * Kernel" <<
                  std::abs(offset) << " +" << std::endl;
        }
        ss << "0.0 ;" << std::endl;
    }
    else if (direction == BlurDirectionBoth) {
        for (size_t i = 0; i < side; i++) {
            int ioffset = (int)(i - radius);
            for (size_t j = 0; j < side; j++) {
                int joffset = (int)(j - radius);
                ss << "texture2D(Texture0, TextureCoord + vec2(" <<
                      ioffset << ".0 * TextureStepX, " <<
                      joffset << ".0 * TextureStepY))" <<
                      " * Kernel" << std::abs(ioffset) <<
                      " * Kernel" << std::abs(joffset) << " +" << std::endl;
            }
        }
        ss << " 0.0;" << std::endl;
    }

    frg_source.replace("$CONVOLUTION$", ss.str());
}

/** 
 * A RenderObject represents a source and target of rendering
 * operations.
 */
class RenderObject
{
public:
    RenderObject() : texture_(0), fbo_(0) { }

    virtual ~RenderObject() { release(); }

    virtual void init()
    {
        /* Create a texture to draw to */
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_.x(), size_.y(), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);

        /* Create a FBO */
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texture_, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /* Load the shader program when this class if first used */
        if (RenderObject::use_count == 0) {
            ShaderSource vtx_source(GLMARK_DATA_PATH"/shaders/desktop.vert");
            ShaderSource frg_source(GLMARK_DATA_PATH"/shaders/desktop.frag");
            Scene::load_shaders_from_strings(main_program, vtx_source.str(),
                                             frg_source.str());
        }

        RenderObject::use_count++;
    }

    virtual void release()
    {
        /* Release resources */
        glDeleteTextures(1, &texture_);
        glDeleteFramebuffers(1, &fbo_);

        /* 
         * Release the shader program when object of this class
         * are no longer in use.
         */
        RenderObject::use_count--;
        if (RenderObject::use_count == 0)
            RenderObject::main_program.release();
    }

    void make_current()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, size_.x(), size_.y());
    }

    void position(const LibMatrix::vec2& pos) { pos_ = pos; }
    const LibMatrix::vec2& position() { return pos_; }


    virtual void size(const LibMatrix::vec2& size)
    {
        /* Recreate the backing texture with correct size */
        if (size_.x() != size.x() || size_.y() != size.y()) {
            size_ = size;
            glBindTexture(GL_TEXTURE_2D, texture_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_.x(), size_.y(), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, 0);
            clear();
        }
    }

    const LibMatrix::vec2& size() { return size_; }

    const LibMatrix::vec2& speed() { return speed_; }
    void speed(const LibMatrix::vec2& speed) { speed_ = speed; }

    GLuint texture() { return texture_; }

    virtual void clear()
    {
        make_current();
        glClear(GL_COLOR_BUFFER_BIT);
    }

    virtual void render_to(RenderObject& target, Program& program = main_program)
    {
        LibMatrix::vec2 final_pos(pos_.x() + size_.x(),
                                   pos_.y() + size_.y());
        LibMatrix::vec2 ll(target.normalize_position(pos_));
        LibMatrix::vec2 ur(target.normalize_position(final_pos));

        GLfloat position[2 * 4] = {
            ll.x(), ll.y(),
            ur.x(), ll.y(),
            ll.x(), ur.y(),
            ur.x(), ur.y(),
        };

        static const GLfloat texcoord[2 * 4] = {
            0.0, 0.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 1.0,
        };

        target.make_current();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);
        draw_quad_with_program(position, texcoord, program);
    }

    virtual void render_from(RenderObject& target, Program& program = main_program)
    {
        LibMatrix::vec2 final_pos(pos_.x() + size_.x(),
                                  pos_.y() + size_.y());
        LibMatrix::vec2 ll_tex(target.normalize_texcoord(pos_));
        LibMatrix::vec2 ur_tex(target.normalize_texcoord(final_pos));

        static const GLfloat position_blur[2 * 4] = {
            -1.0, -1.0,
             1.0, -1.0,
            -1.0,  1.0,
             1.0,  1.0,
        };
        GLfloat texcoord_blur[2 * 4] = {
            ll_tex.x(), ll_tex.y(),
            ur_tex.x(), ll_tex.y(),
            ll_tex.x(), ur_tex.y(),
            ur_tex.x(), ur_tex.y(),
        };
            
        make_current();
        glBindTexture(GL_TEXTURE_2D, target.texture());
        draw_quad_with_program(position_blur, texcoord_blur, program);
    }

    /** 
     * Normalizes a position from [0, size] to [-1.0, 1.0]
     */
    LibMatrix::vec2 normalize_position(LibMatrix::vec2& pos)
    {
        return LibMatrix::vec2(2.0 * pos.x() / size_.x() - 1.0,
                               2.0 * pos.y() / size_.y() - 1.0);
    }

    /** 
     * Normalizes a position from [0, size] to [0.0, 1.0]
     */
    LibMatrix::vec2 normalize_texcoord(LibMatrix::vec2& pos)
    {
        return LibMatrix::vec2(((float)pos.x()) / size_.x(),
                               ((float)pos.y()) / size_.y());
    }


protected:
    void draw_quad_with_program(const GLfloat *position, const GLfloat *texcoord,
                                Program &program)
    {
        int pos_index = program["position"].location();
        int tex_index = program["texcoord"].location();

        program.start();

        glEnableVertexAttribArray(pos_index);
        glEnableVertexAttribArray(tex_index);
        glVertexAttribPointer(pos_index, 2,
                              GL_FLOAT, GL_FALSE, 0, position);
        glVertexAttribPointer(tex_index, 2,
                              GL_FLOAT, GL_FALSE, 0, texcoord);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(tex_index);
        glDisableVertexAttribArray(pos_index);

        program.stop();
    }

    static Program main_program;

    LibMatrix::vec2 pos_;
    LibMatrix::vec2 size_;
    LibMatrix::vec2 speed_;
    GLuint texture_;
    GLuint fbo_;

private:
    static int use_count;

};

int RenderObject::use_count = 0;
Program RenderObject::main_program;

/** 
 * A RenderObject representing the screen.
 *
 * Rendering to this objects renders to the screen framebuffer.
 */
class RenderScreen : public RenderObject
{
public:
    virtual void init() {}
};

/** 
 * A RenderObject with a background image.
 *
 * The image is drawn to the RenderObject automatically when the
 * object is cleared, resized etc
 */
class RenderClearImage : public RenderObject
{
public:
    RenderClearImage(const std::string& texture) :
        RenderObject(), background_texture_name(texture),
        background_texture_(0) {}

    virtual void init()
    {
        RenderObject::init();

        /* Load the image into a texture */
        Texture::load(background_texture_name,
                      &background_texture_, GL_LINEAR, GL_LINEAR, 0);

    }

    virtual void release()
    {
        glDeleteTextures(1, &background_texture_);
        RenderObject::release();
    }

    virtual void clear()
    {
        static const GLfloat position[2 * 4] = {
            -1.0, -1.0,
             1.0, -1.0,
            -1.0,  1.0,
             1.0,  1.0,
        };
        static const GLfloat texcoord[2 * 4] = {
            0.0, 0.0,
            1.0, 0.0,
            0.0, 1.0,
            1.0, 1.0,
        };

        make_current();
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, background_texture_);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        draw_quad_with_program(position, texcoord, main_program);
        glDisable(GL_BLEND);
    }

private:
    std::string background_texture_name;
    GLuint background_texture_;
};

/** 
 * A RenderObject that blurs the target it is drawn to.
 */
class RenderWindowBlur : public RenderObject
{
public:
    RenderWindowBlur(unsigned int passes, unsigned int radius, bool separable,
                     bool draw_contents = true) :
        RenderObject(), passes_(passes), radius_(radius), separable_(separable),
        draw_contents_(draw_contents) {}

    virtual void init()
    {
        RenderObject::init();

        /* Only have one instance of the window contents data */
        if (draw_contents_ && RenderWindowBlur::use_count == 0)
            window_contents_.init();

        RenderWindowBlur::use_count++; 
    }

    virtual void release()
    {
        RenderWindowBlur::use_count--;

        /* Only have one instance of the window contents data */
        if (draw_contents_ && RenderWindowBlur::use_count == 0)
            window_contents_.release();

        RenderObject::release();
    }

    virtual void size(const LibMatrix::vec2& size)
    {
        RenderObject::size(size);
        if (draw_contents_)
            window_contents_.size(size);
    }

    virtual void render_to(RenderObject& target, Program& program)
    {
        (void)program;

        if (separable_) {
            Program& blur_program_h1 = blur_program_h(target.size().x());
            Program& blur_program_v1 = blur_program_v(target.size().y());

            for (unsigned int i = 0; i < passes_; i++) {
                render_from(target, blur_program_h1);
                RenderObject::render_to(target, blur_program_v1);
            }
        }
        else {
            Program& blur_program1 = blur_program(target.size().x(), target.size().y());

            for (unsigned int i = 0; i < passes_; i++) {
                if (i % 2 == 0)
                    render_from(target, blur_program1);
                else 
                    RenderObject::render_to(target, blur_program1);
            }

            if (passes_ % 2 == 1)
                RenderObject::render_to(target);
        }

        /* 
         * Blend the window contents with the target texture.
         */
        if (draw_contents_) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            window_contents_.position(position());
            window_contents_.render_to(target);
            glDisable(GL_BLEND);
        }
    }

private:
    Program& blur_program(unsigned int w, unsigned int h)
    {
        /* 
         * If the size of the window has changed we must recreate
         * the shader to contain the correct texture step values.
         */
        if (blur_program_dim_.x() != w || blur_program_dim_.y() != h ||
            !blur_program_.ready())
        {
            blur_program_dim_.x(w);
            blur_program_dim_.y(h);

            blur_program_.release();

            ShaderSource vtx_source;
            ShaderSource frg_source;
            create_blur_shaders(vtx_source, frg_source, radius_,
                                radius_ / 3.0, BlurDirectionBoth);
            frg_source.add_const("TextureStepX", 1.0 / w);
            frg_source.add_const("TextureStepY", 1.0 / h);
            Scene::load_shaders_from_strings(blur_program_, vtx_source.str(),
                                             frg_source.str());
        }

        return blur_program_;
    }

    Program& blur_program_h(unsigned int w)
    {
        /* 
         * If the size of the window has changed we must recreate
         * the shader to contain the correct texture step values.
         */
        if (blur_program_dim_.x() != w ||
            !blur_program_h_.ready())
        {
            blur_program_dim_.x(w);

            blur_program_h_.release();

            ShaderSource vtx_source;
            ShaderSource frg_source;
            create_blur_shaders(vtx_source, frg_source, radius_,
                                radius_ / 3.0, BlurDirectionHorizontal);
            frg_source.add_const("TextureStepX", 1.0 / w);
            Scene::load_shaders_from_strings(blur_program_h_, vtx_source.str(),
                                             frg_source.str());
        }

        return blur_program_h_;
    }

    Program& blur_program_v(unsigned int h)
    {
        /* 
         * If the size of the window has changed we must recreate
         * the shader to contain the correct texture step values.
         */
        if (blur_program_dim_.y() != h ||
            !blur_program_v_.ready())
        {
            blur_program_dim_.y(h);

            blur_program_v_.release();

            ShaderSource vtx_source;
            ShaderSource frg_source;
            create_blur_shaders(vtx_source, frg_source, radius_,
                                radius_ / 3.0, BlurDirectionVertical);
            frg_source.add_const("TextureStepY", 1.0 / h);
            Scene::load_shaders_from_strings(blur_program_v_, vtx_source.str(),
                                             frg_source.str());
        }

        return blur_program_v_;
    }

    LibMatrix::uvec2 blur_program_dim_;
    Program blur_program_;
    Program blur_program_h_;
    Program blur_program_v_;
    unsigned int passes_;
    unsigned int radius_;
    bool separable_;
    bool draw_contents_;

    static int use_count;
    static RenderClearImage window_contents_;

};

int RenderWindowBlur::use_count = 0;
RenderClearImage RenderWindowBlur::window_contents_(GLMARK_DATA_PATH"/textures/desktop-window.png");

/*******************************
 * SceneDesktop implementation * 
 *******************************/

/** 
 * Private structure used to avoid contaminating scene.h with all of the
 * SceneDesktop internal classes.
 */
struct SceneDesktopPrivate
{
    RenderScreen screen;
    RenderClearImage desktop;
    std::vector<RenderObject *> windows;

    SceneDesktopPrivate() :
        desktop(GLMARK_DATA_PATH"/textures/effect-2d.png") {}

    ~SceneDesktopPrivate() { Util::dispose_pointer_vector(windows); }
    
};


SceneDesktop::SceneDesktop(Canvas &canvas) :
    Scene(canvas, "desktop")
{
    priv_ = new SceneDesktopPrivate();
    mOptions["effect"] = Scene::Option("effect", "blur",
                                       "the effect to use [blur]");
    mOptions["windows"] = Scene::Option("windows", "4",
                                        "the number of windows");
    mOptions["window-size"] = Scene::Option("window-size", "0.35",
                                            "the window size as a percentage of the minimum screen dimension [0.0 - 0.5]");
    mOptions["passes"] = Scene::Option("passes", "1",
                                       "the number of effect passes (effect dependent)");
    mOptions["blur-radius"] = Scene::Option("blur-radius", "5",
                                            "the blur effect radius (in pixels)");
    mOptions["separable"] = Scene::Option("separable", "true",
                                          "use separable convolution for the blur effect");
}

SceneDesktop::~SceneDesktop()
{
    delete priv_;
}

int
SceneDesktop::load()
{
    priv_->screen.init();
    priv_->desktop.init();
    return 1;
}

void
SceneDesktop::unload()
{
    priv_->desktop.release();
    priv_->screen.release();
}

void
SceneDesktop::setup()
{
    Scene::setup();

    std::stringstream ss;
    unsigned int windows(0);
    unsigned int passes(0);
    unsigned int blur_radius(0);
    float window_size_factor(0.0);
    bool separable(mOptions["separable"].value == "true");

    ss << mOptions["windows"].value;
    ss >> windows;
    ss.clear();
    ss << mOptions["window-size"].value;
    ss >> window_size_factor;
    ss.clear();
    ss << mOptions["passes"].value;
    ss >> passes;
    ss.clear();
    ss << mOptions["blur-radius"].value;
    ss >> blur_radius;

    /* Ensure we get a transparent clear color for all following operations */
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    /* Set up the screen and desktop RenderObjects */
    priv_->screen.size(LibMatrix::vec2(mCanvas.width(), mCanvas.height()));
    priv_->desktop.size(LibMatrix::vec2(mCanvas.width(), mCanvas.height()));

    /* Create the windows */
    float angular_step(2.0 * M_PI / windows);
    unsigned int min_dimension = std::min(mCanvas.width(), mCanvas.height());
    float window_size(min_dimension * window_size_factor);
    static const LibMatrix::vec2 corner_offset(window_size / 2.0,
                                               window_size / 2.0);

    for (unsigned int i = 0; i < windows; i++) {
        LibMatrix::vec2 center(mCanvas.width() * (0.5 + 0.25 * cos(i * angular_step)),
                               mCanvas.height() * (0.5 + 0.25 * sin(i * angular_step)));
        RenderObject* win(new RenderWindowBlur(passes, blur_radius, separable));
        (void)angular_step;

        win->init();
        win->position(center - corner_offset);
        win->size(LibMatrix::vec2(window_size, window_size));
        /* 
         * Set the speed in increments of about 30 degrees (but not exactly,
         * so we don't get windows moving just on the X axis or Y axis).
         */
        win->speed(LibMatrix::vec2(cos(0.1 + i * M_PI / 6.0) * mCanvas.width() / 3,
                                   sin(0.1 + i * M_PI / 6.0) * mCanvas.height() / 3));
        /* 
         * Perform a dummy rendering to ensure internal shaders are initialized
         * now, in order not to affect the benchmarking.
         */
        win->render_to(priv_->desktop);
        priv_->windows.push_back(win);
    }

    /* 
     * Ensure the screen is the current rendering target (it might have changed
     * to a FBO in the previous steps).
     */
    priv_->screen.make_current();

    mCurrentFrame = 0;
    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}

void
SceneDesktop::teardown()
{
    Util::dispose_pointer_vector(priv_->windows);
    priv_->screen.make_current();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    Scene::teardown();
}

void
SceneDesktop::update()
{
    double current_time = Scene::get_timestamp_us() / 1000000.0;
    double dt = current_time - mLastUpdateTime;
    double elapsed_time = current_time - mStartTime;

    mLastUpdateTime = current_time;

    std::vector<RenderObject *>& windows(priv_->windows);

    /*
     * Move the windows around the screen, bouncing them back when
     * they reach the edge.
     */
    for (std::vector<RenderObject *>::const_iterator iter = windows.begin();
         iter != windows.end();
         iter++)
    {
        bool should_update = true;
        RenderObject *win = *iter;
        LibMatrix::vec2 new_pos(
                win->position().x() + win->speed().x() * dt,
                win->position().y() + win->speed().y() * dt);

        if (new_pos.x() < 0.0 ||
            new_pos.x() + win->size().x() > ((float)mCanvas.width()))
        {
            win->speed(LibMatrix::vec2(-win->speed().x(), win->speed().y()));
            should_update = false;
        }

        if (new_pos.y() < 0.0 ||
            new_pos.y() + win->size().y() > ((float)mCanvas.height()))
        {
            win->speed(LibMatrix::vec2(win->speed().x(), -win->speed().y()));
            should_update = false;
        }

        if (should_update)
            win->position(new_pos);
    }

    if (elapsed_time >= mDuration) {
        mAverageFPS = mCurrentFrame / elapsed_time;
        mRunning = false;
    }

    mCurrentFrame++;
}

void
SceneDesktop::draw()
{
    std::vector<RenderObject *>& windows(priv_->windows);

    /* Ensure we get a transparent clear color for all following operations */
    glClearColor(0.0, 0.0, 0.0, 0.0);

    priv_->desktop.clear();

    for (std::vector<RenderObject *>::const_iterator iter = windows.begin();
         iter != windows.end();
         iter++)
    {
        RenderObject *win = *iter;
        win->render_to(priv_->desktop);
    }

    priv_->desktop.render_to(priv_->screen);

}

Scene::ValidationResult
SceneDesktop::validate()
{
    return ValidationUnknown;
}
