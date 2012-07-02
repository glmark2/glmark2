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
//  Aleksandar Rodic - Creator and WebGL implementation 
//  Jesse Barker - glmark2 port
//
#include "scene.h"
#include "jellyfish-private.h"
#include "util.h"

SceneJellyfish::SceneJellyfish(Canvas& canvas) :
    Scene(canvas, "jellyfish")
{

}

SceneJellyfish::~SceneJellyfish()
{
    delete priv_;
}

bool
SceneJellyfish::load()
{
    running_ = false;
    return true;
}

void
SceneJellyfish::unload()
{
}

void
SceneJellyfish::setup()
{
    // Core Scene state
    Scene::setup();
    uint64_t now = Util::get_timestamp_us();
    startTime_ = now / 1000000.0;
    lastUpdateTime_ = startTime_;
    running_ = true;

    // Set up our private object that does all of the lifting
    priv_ = new JellyfishPrivate();
    priv_->initialize(now);
}

void
SceneJellyfish::teardown()
{
    priv_->cleanup();
    Scene::teardown();
}

void
SceneJellyfish::update()
{
    Scene::update();
    priv_->update_viewport(LibMatrix::vec2(canvas_.width(), canvas_.height()));
    priv_->update_time();
}

void
SceneJellyfish::draw()
{
    priv_->draw();
}

Scene::ValidationResult
SceneJellyfish::validate()
{
    return Scene::ValidationUnknown;
}
