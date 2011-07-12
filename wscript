import commands
import subprocess
import os
import Options
import Scripting

out = 'build'
top = '.'

VERSION = '2011.06'
APPNAME = 'glmark2'

def options(opt):
    opt.tool_options('compiler_cc')
    opt.tool_options('compiler_cxx')

    opt.add_option('--enable-gl', action='store_true', dest = 'gl',
                   default = False, help='build using OpenGL 2.0')
    opt.add_option('--enable-glesv2', action='store_true', dest = 'glesv2',
                   default = False, help='build using OpenGL ES 2.0')
    opt.add_option('--no-debug', action='store_false', dest = 'debug',
                   default = True, help='disable compiler debug information')
    opt.add_option('--no-opt', action='store_false', dest = 'opt',
                   default = True, help='disable compiler optimizations')
    opt.add_option('--data-path', action='store', dest = 'data_path',
                   help='the path to install the data to')

def configure(ctx):
    if not Options.options.gl and not Options.options.glesv2:
        ctx.fatal("You must configure using at least one of --enable-gl, --enable-glesv2")

    ctx.check_tool('compiler_cc')
    ctx.check_tool('compiler_cxx')

    # Check required headers
    req_headers = ['stdlib.h', 'string.h', 'unistd.h', 'fcntl.h']
    for header in req_headers:
        ctx.check_cxx(header_name = header, mandatory = True)

    # Check for required libs
    req_libs = [('m', 'm')]
    for (lib, uselib) in req_libs:
        ctx.check_cxx(lib = lib, uselib_store = uselib)

    # Check required functions
    req_funcs = [('memset', 'string.h', []) ,('sqrt', 'math.h', ['m'])]
    for func, header, uselib in req_funcs:
        ctx.check_cxx(function_name = func, header_name = header,
                      uselib = uselib, mandatory = True)

    # Check required packages
    req_pkgs = [('x11', 'x11'), ('libpng12', 'libpng12')]
    for (pkg, uselib) in req_pkgs:
        ctx.check_cfg(package = pkg, uselib_store = uselib,
                      args = '--cflags --libs', mandatory = True)

    # Check optional packages
    opt_pkgs = [('gl', 'gl', Options.options.gl),
                ('egl', 'egl', Options.options.glesv2),
                ('glesv2', 'glesv2', Options.options.glesv2)]
    for (pkg, uselib, mandatory) in opt_pkgs:
        ctx.check_cfg(package = pkg, uselib_store = uselib,
                      args = '--cflags --libs', mandatory = mandatory)

    ctx.env.append_unique('CXXFLAGS', '-Wall -Werror -Wextra -fms-extensions'.split(' '))

    # Prepend -O# and -g flags so that they can be overriden by the
    # CFLAGS environment variable
    if Options.options.opt:
        ctx.env.prepend_value('CXXFLAGS', '-O2')
    if Options.options.debug:
        ctx.env.prepend_value('CXXFLAGS', '-g')

    if Options.options.data_path is None:
        Options.options.data_path = os.path.join(ctx.env.PREFIX, 'share/glmark2')

    ctx.env.append_unique('GLMARK_DATA_PATH', Options.options.data_path)
    ctx.env.append_unique('DEFINES', 'GLMARK_DATA_PATH="%s"' % Options.options.data_path)
    ctx.env.append_unique('DEFINES', 'GLMARK_VERSION="%s"' % VERSION)
    ctx.env.GLMARK2_VERSION = VERSION

    ctx.env.USE_GL = Options.options.gl
    ctx.env.USE_GLESv2 = Options.options.glesv2

    ctx.msg("Prefix", ctx.env.PREFIX, color = 'PINK')
    ctx.msg("Data path", Options.options.data_path, color = 'PINK')
    ctx.msg("Building GL2 version", "Yes" if ctx.env.USE_GL else "No",
            color = 'PINK')
    ctx.msg("Building GLESv2 version", "Yes" if ctx.env.USE_GLESv2 else "No",
            color = 'PINK')

def build(ctx):
    ctx.recurse('src')
    ctx.recurse('data')
    ctx.recurse('doc')

def dist(ctx):
    ctx.algo = 'tar.gz'
