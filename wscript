import commands
import subprocess
import os
import Options
import Scripting
from waflib import Context

out = 'build'
top = '.'

VERSION = '2012.12'
APPNAME = 'glmark2'

def options(opt):
    opt.tool_options('gnu_dirs')
    opt.tool_options('compiler_cc')
    opt.tool_options('compiler_cxx')

    opt.add_option('--enable-gl', action='store_true', dest = 'gl',
                   default = False, help='build using OpenGL 2.0')
    opt.add_option('--enable-glesv2', action='store_true', dest = 'glesv2',
                   default = False, help='build using OpenGL ES 2.0')
    opt.add_option('--enable-gl-drm', action='store_true', dest = 'gl_drm',
                   default = False, help='build using OpenGL 2.0 without X')
    opt.add_option('--enable-glesv2-drm', action='store_true',
                   dest = 'glesv2_drm',
                   default = False, help='build using OpenGL ES 2.0 without X')
    opt.add_option('--no-debug', action='store_false', dest = 'debug',
                   default = True, help='disable compiler debug information')
    opt.add_option('--no-opt', action='store_false', dest = 'opt',
                   default = True, help='disable compiler optimizations')
    opt.add_option('--data-path', action='store', dest = 'data_path',
                   help='path to main data (also see --data(root)dir)')
    opt.add_option('--extras-path', action='store', dest = 'extras_path',
                   help='path to additional data (models, shaders, textures)')

def configure(ctx):
    if not Options.options.gl and not Options.options.glesv2 and \
       not Options.options.gl_drm and not Options.options.glesv2_drm:
        ctx.fatal("You must configure using at least one of --enable-gl, " +
                  "--enable-glesv2, --enable-gl-drm, --enable-glesv2-drm")

    ctx.check_tool('gnu_dirs')
    ctx.check_tool('compiler_cc')
    ctx.check_tool('compiler_cxx')

    # Check required headers
    req_headers = ['stdlib.h', 'string.h', 'unistd.h', 'stdint.h', 'stdio.h', 'jpeglib.h']
    for header in req_headers:
        ctx.check_cxx(header_name = header, auto_add_header_name = True, mandatory = True)

    # Check for required libs
    req_libs = [('m', 'm'), ('jpeg', 'jpeg')]
    for (lib, uselib) in req_libs:
        ctx.check_cxx(lib = lib, uselib_store = uselib)

    # Check required functions
    req_funcs = [('memset', 'string.h', []) ,('sqrt', 'math.h', ['m'])]
    for func, header, uselib in req_funcs:
        ctx.check_cxx(function_name = func, header_name = header,
                      uselib = uselib, mandatory = True)

    # Check required packages
    req_pkgs = [('libpng12', 'libpng12')]
    for (pkg, uselib) in req_pkgs:
        ctx.check_cfg(package = pkg, uselib_store = uselib,
                      args = '--cflags --libs', mandatory = True)

    # Check optional packages
    opt_pkgs = [('x11', 'x11', Options.options.gl or Options.options.glesv2),
                ('gl', 'gl', Options.options.gl or Options.options.gl_drm),
                ('egl', 'egl', Options.options.glesv2 or
                               Options.options.glesv2_drm),
                ('glesv2', 'glesv2', Options.options.glesv2 or
                                     Options.options.glesv2_drm),
                ('libdrm','drm', Options.options.gl_drm or
                                 Options.options.glesv2_drm),
                ('gbm','gbm', Options.options.gl_drm or
                              Options.options.glesv2_drm)]
    for (pkg, uselib, mandatory) in opt_pkgs:
        ctx.check_cfg(package = pkg, uselib_store = uselib,
                      args = '--cflags --libs', mandatory = mandatory)

    ctx.env.append_unique('CXXFLAGS', '-Werror -Wall -Wextra -Wnon-virtual-dtor'.split(' '))

    # Prepend -O# and -g flags so that they can be overriden by the
    # CFLAGS environment variable
    if Options.options.opt:
        ctx.env.prepend_value('CXXFLAGS', '-O2')
    if Options.options.debug:
        ctx.env.prepend_value('CXXFLAGS', '-g')

    ctx.env.HAVE_EXTRAS = False
    if Options.options.extras_path is not None:
        ctx.env.HAVE_EXTRAS = True
        ctx.env.append_unique('GLMARK_EXTRAS_PATH', Options.options.extras_path)
        ctx.env.append_unique('DEFINES', 'GLMARK_EXTRAS_PATH="%s"' % Options.options.extras_path)

    if Options.options.data_path is not None:
        data_path = Options.options.data_path 
    else:
        data_path = os.path.join(ctx.env.DATADIR, 'glmark2')

    ctx.env.append_unique('GLMARK_DATA_PATH', data_path)
    ctx.env.append_unique('DEFINES', 'GLMARK_DATA_PATH="%s"' % data_path)
    ctx.env.append_unique('DEFINES', 'GLMARK_VERSION="%s"' % VERSION)
    ctx.env.GLMARK2_VERSION = VERSION

    ctx.env.USE_GL = Options.options.gl
    ctx.env.USE_GLESv2 = Options.options.glesv2
    ctx.env.USE_GL_DRM = Options.options.gl_drm
    ctx.env.USE_GLESv2_DRM = Options.options.glesv2_drm

    ctx.msg("Prefix", ctx.env.PREFIX, color = 'PINK')
    ctx.msg("Data path", data_path, color = 'PINK')
    ctx.msg("Including extras", "Yes" if ctx.env.HAVE_EXTRAS else "No",
            color = 'PINK');
    if ctx.env.HAVE_EXTRAS:
        ctx.msg("Extras path", Options.options.extras_path, color = 'PINK')
    ctx.msg("Building X11 GL2 version", "Yes" if ctx.env.USE_GL else "No",
            color = 'PINK')
    ctx.msg("Building X11 GLESv2 version", "Yes" if ctx.env.USE_GLESv2 else "No",
            color = 'PINK')
    ctx.msg("Building DRM GL2 version", "Yes" if ctx.env.USE_GL_DRM else "No",
            color = 'PINK')
    ctx.msg("Building DRM GLESv2 version", "Yes" if ctx.env.USE_GLESv2_DRM else "No",
            color = 'PINK')

def build(ctx):
    ctx.recurse('src')
    ctx.recurse('data')
    ctx.recurse('doc')

class Glmark2Dist(Context.Context):
    """ Custom dist command that preserves symbolic links"""

    cmd = "dist"

    def execute(self):
        self.recurse([os.path.dirname(Context.g_module.root_path)])
        self.archive()

    def get_files(self):
        import fnmatch
        files = []
        excludes = ['*.bzr', '*~', './.*waf*', './build*', '*.swp', '*.pyc', '*glmark2-*.tar.gz']
        for (dirpath, dirnames, filenames) in os.walk(top):
            names_to_remove = []
            names = dirnames + filenames
            for n in names:
                for exclude in excludes:
                    if fnmatch.fnmatch(os.path.join(dirpath, n), exclude):
                        names_to_remove.append(n)
                        break

            for d in names_to_remove:
                if d in dirnames:
                    dirnames.remove(d)
                if d in filenames:
                    filenames.remove(d)

            files.extend([os.path.join(dirpath, d) for d in dirnames])
            files.extend([os.path.join(dirpath, f) for f in filenames])

        return files

    def archive(self):
        import tarfile
        tar = tarfile.open(APPNAME + '-' + VERSION + '.tar.gz', 'w:gz')
        for f in self.get_files():
            tar.add(f, arcname = APPNAME + '-' + VERSION + '/' + f, recursive = False)
        tar.close()
