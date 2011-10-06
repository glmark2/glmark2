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
#include "scene.h"
#include "log.h"
#include "mat.h"
#include "stack.h"
#include "shader-source.h"
#include "util.h"
#include <cmath>

/**************************************
 * TransformableObject implementation *
 **************************************/

/** 
 * An object that changes its shape as a function of elapsed time,
 * moving between a series of states using linear interpolation.
 */
class TransformableObject
{
public:
    /** 
     * Attach a Transformable object to a mesh.
     */
    void attach_to_mesh(Mesh& mesh)
    {
        mesh_range_.first = mesh.vertices().size();

        for (size_t i = 0; i < vertex_index_.size(); i++) {
            size_t n = (i / 3) * 3;

            mesh.next_vertex();
            mesh.set_attrib(0, current_state_data_[vertex_index_[i]]);
            mesh.set_attrib(1, current_state_data_[vertex_index_[n]]);
            mesh.set_attrib(2, current_state_data_[vertex_index_[n + 1]]);
            mesh.set_attrib(3, current_state_data_[vertex_index_[n + 2]]);
        }

        mesh_range_.second = mesh.vertices().size() - 1;
    }

    /** 
     * Updates the mesh data with the current object state.
     */
    std::pair<size_t,size_t>& update_mesh(Mesh& mesh)
    {
        for (size_t i = 0; i < vertex_index_.size(); i++) {
            size_t n = (i / 3) * 3;

            std::vector<float> *vertex( &mesh.vertices()[i + mesh_range_.first]);

            mesh.set_attrib(0, current_state_data_[vertex_index_[i]],
                            vertex);
            mesh.set_attrib(1, current_state_data_[vertex_index_[n]],
                            vertex);
            mesh.set_attrib(2, current_state_data_[vertex_index_[n + 1]],
                            vertex);
            mesh.set_attrib(3, current_state_data_[vertex_index_[n + 2]],
                            vertex);
        }

        return mesh_range_;
    }

    /** 
     * Updates the object state.
     *
     * @param elapsed the elapsed time since the start of the
     *        transformation process
     */
    void update(double elapsed)
    {
        double transition_time(period_ / (nstates_ - 1));
        int state_changes(static_cast<int>(elapsed / transition_time));
        int state((start_state_ + state_changes) % nstates_);
        int state_next((state + 1) % nstates_);
        double factor(std::fmod(elapsed, transition_time) / transition_time);

        current_state_data_.assign_lerp(state_data_[state],
                                        state_data_[state_next],
                                        factor);
    }

    /** 
     * The time it should take to perform a full transformation sequence.
     */
    void period(double t) { period_ = t; }

    /** 
     * A vector containing vertices (just vec3 for now), with additional
     * support for linear interpolation.
     */
    class VertexVector : public std::vector<LibMatrix::vec3>
    {
    public:
        VertexVector() {}

        VertexVector& assign_lerp(const VertexVector& a, const VertexVector& b,
                                  double factor)
        {
            size_t min_size = std::min(std::min(a.size(), b.size()), size());
            for (size_t i = 0; i < min_size; i++) {
                (*this)[i] = a[i] + (b[i] - a[i]) * factor;
            }

            return *this;
        }
    };

protected:
    TransformableObject() :
        period_(10.0), start_state_(0), nstates_(0)
    {
    }

    double period_;
    size_t start_state_;
    size_t nstates_;
    VertexVector current_state_data_;
    std::vector<VertexVector> state_data_;
    std::vector<int> vertex_index_;
    std::pair<size_t,size_t> mesh_range_;
};

/** 
 * An object that changes between a tetrahedron and a cube.
 */
class TransformableCube : public TransformableObject
{
public:
    TransformableCube(const LibMatrix::mat4& transform = LibMatrix::mat4())
    {
        nstates_ = 2;
        state_data_.resize(nstates_);
        period_ = 2.5;

        static const LibMatrix::vec4 vertices[] = {
            LibMatrix::vec4(-1.0, -1.0,  1.0, 1.0),
            LibMatrix::vec4( 1.0, -1.0,  1.0, 1.0),
            LibMatrix::vec4(-1.0,  1.0,  1.0, 1.0),
            LibMatrix::vec4( 1.0,  1.0,  1.0, 1.0),
            LibMatrix::vec4(-1.0, -1.0, -1.0, 1.0),
            LibMatrix::vec4( 1.0, -1.0, -1.0, 1.0),
            LibMatrix::vec4(-1.0,  1.0, -1.0, 1.0),
            LibMatrix::vec4( 1.0,  1.0, -1.0, 1.0)
        };
        
        /* Fill in the initial state (possibly transformed) */
        size_t nvertices = sizeof(vertices) / sizeof(*vertices);
        for (size_t i = 0; i < nvertices; i++) {
            const LibMatrix::vec4& tvertex4(transform * vertices[i]);
            LibMatrix::vec3 tvertex3(tvertex4.x(), tvertex4.y(), tvertex4.z());
            state_data_[0].push_back(tvertex3);
        }

        /* Fill in the second state */
        state_data_[1].push_back((state_data_[0][1] +
                                  state_data_[0][2] +
                                  state_data_[0][4]) / 3.0);
        state_data_[1].push_back(state_data_[0][1]);
        state_data_[1].push_back(state_data_[0][2]);
        state_data_[1].push_back((state_data_[0][1] +
                                  state_data_[0][2] +
                                  state_data_[0][7]) / 3.0);

        state_data_[1].push_back(state_data_[0][4]);
        state_data_[1].push_back((state_data_[0][1] +
                                  state_data_[0][4] +
                                  state_data_[0][7]) / 3.0);
        state_data_[1].push_back((state_data_[0][2] +
                                  state_data_[0][4] +
                                  state_data_[0][7]) / 3.0);
        state_data_[1].push_back(state_data_[0][7]);

        static const int indices[] = {
            0, 1, 2, 2, 1, 3, /* front */
            5, 4, 7, 7, 4, 6, /* back */
            3, 1, 7, 7, 1, 5, /* right */
            6, 4, 2, 2, 4, 0, /* left */
            6, 2, 7, 7, 2, 3, /* top */
            0, 4, 1, 1, 4, 5, /* bottom */
        };

        vertex_index_.assign(indices, indices + sizeof(indices) / sizeof(*indices));

        current_state_data_ = state_data_[start_state_];
    }

};

/******************************
 * SceneBuffer implementation *
 ******************************/

struct SceneBufferPrivate {
    std::vector<TransformableObject *> objects;
    ~SceneBufferPrivate() { Util::dispose_pointer_vector(objects); }
};

SceneBuffer::SceneBuffer(Canvas &pCanvas) :
    Scene(pCanvas, "buffer")
{
    priv_ = new SceneBufferPrivate();
    mOptions["interleave"] = Scene::Option("interleave", "false",
                                           "Whether to interleave vertex attribute data [true,false]");
    mOptions["update-method"] = Scene::Option("update-method", "map",
                                              "[map,subdata]");
}

SceneBuffer::~SceneBuffer()
{
    delete priv_;
}

int SceneBuffer::load()
{
    mRunning = false;

    return 1;
}

void SceneBuffer::unload()
{
    mMesh.reset();

}

void SceneBuffer::setup()
{
    using LibMatrix::vec3;

    Scene::setup();

    bool interleave = (mOptions["interleave"].value == "true");
    Mesh::VBOUpdateMethod update_method;

    if (mOptions["update-method"].value == "map")
        update_method = Mesh::VBOUpdateMethodMap;
    else if (mOptions["update-method"].value == "subdata")
        update_method = Mesh::VBOUpdateMethodSubData;
    else
        update_method = Mesh::VBOUpdateMethodMap;

    /* Set up shaders */
    static const std::string vtx_shader_filename(GLMARK_DATA_PATH"/shaders/buffer-wireframe.vert");
    static const std::string frg_shader_filename(GLMARK_DATA_PATH"/shaders/buffer-wireframe.frag");

    ShaderSource vtx_source(vtx_shader_filename);
    ShaderSource frg_source(frg_shader_filename);

    if (!Scene::load_shaders_from_strings(mProgram, vtx_source.str(),
                                          frg_source.str()))
    {
        return;
    }

    /* 
     * We need to pass the positions of all vertex of the triangle
     * in order to draw the wireframe.
     */
    std::vector<int> vertex_format;
    vertex_format.push_back(3);     // Position of vertex
    vertex_format.push_back(3);     // Position of triangle vertex 0
    vertex_format.push_back(3);     // Position of triangle vertex 1
    vertex_format.push_back(3);     // Position of triangle vertex 2
    mMesh.set_vertex_format(vertex_format);

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram["position"].location());
    attrib_locations.push_back(mProgram["tvertex0"].location());
    attrib_locations.push_back(mProgram["tvertex1"].location());
    attrib_locations.push_back(mProgram["tvertex2"].location());
    mMesh.set_attrib_locations(attrib_locations);

    /* Add objects */
    priv_->objects.push_back(new TransformableCube());

    for (std::vector<TransformableObject*>::iterator si = priv_->objects.begin();
         si != priv_->objects.end();
         si++)
    {
        TransformableObject *object(*si);

        object->attach_to_mesh(mMesh);
    }

    mMesh.vbo_update_method(update_method);
    mMesh.interleave(interleave);

    mMesh.build_vbo();

    mProgram.start();
    mProgram["Viewport"] = LibMatrix::vec2(mCanvas.width(), mCanvas.height());

    /* Enable alpha blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mCurrentFrame = 0;
    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}

void
SceneBuffer::teardown()
{
    Util::dispose_pointer_vector(priv_->objects);
    mProgram.stop();
    mProgram.release();

    mMesh.reset();

    /* Reset default values */
    glDisable(GL_BLEND);
    glCullFace(GL_BACK);

    Scene::teardown();
}

void SceneBuffer::update()
{
    double current_time = Scene::get_timestamp_us() / 1000000.0;
    double elapsed_time = current_time - mStartTime;

    mLastUpdateTime = current_time;

    if (elapsed_time >= mDuration) {
        mAverageFPS = mCurrentFrame / elapsed_time;
        mRunning = false;
    }

    std::vector<std::pair<size_t,size_t> > ranges;
    for (std::vector<TransformableObject*>::iterator si = priv_->objects.begin();
         si != priv_->objects.end();
         si++)
    {
        TransformableObject *object(*si);

        object->update(elapsed_time);
        ranges.push_back(object->update_mesh(mMesh));
    }

    mMesh.update_vbo(ranges);

    mCurrentFrame++;
}

void SceneBuffer::draw()
{
    LibMatrix::Stack4 model_view;

    // Load the ModelViewProjectionMatrix uniform in the shader
    LibMatrix::mat4 model_view_proj(mCanvas.projection());
    model_view.translate(0.0f, 0.0f, -5.0f);
    model_view_proj *= model_view.getCurrent();

    mProgram["ModelViewProjectionMatrix"] = model_view_proj;

    /* 
     * Render the back faces first and then the front faces to produce
     * a correct translucency effect (this works because our objects
     * are convex).
     */
    glCullFace(GL_FRONT);
    mMesh.render_vbo();

    glCullFace(GL_BACK);
    mMesh.render_vbo();
}

Scene::ValidationResult
SceneBuffer::validate()
{
    return Scene::ValidationUnknown;
}
