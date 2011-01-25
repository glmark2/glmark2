import commands
import subprocess
import os
import Options
import Scripting

blddir = 'build'

top = '.'
VERSION = '10.07.1'
APPNAME = 'glmark2'

# Produce '.tar.gz' with ./waf dist
Scripting.g_gz = 'gz'

def conf_message(conf, first, second):
	conf.check_message_1(first)
	conf.check_message_2(second, color='PINK')

def set_options(opt):
	opt.tool_options('compiler_cc')
	opt.tool_options('compiler_cxx')

	opt.add_option('--enable-gl', action='store_true', dest = 'gl', default = False, help='build using OpenGL 2.0')
	opt.add_option('--enable-glesv2', action='store_true', dest = 'glesv2', default = False, help='build using OpenGL ES 2.0')
	opt.add_option('--no-debug', action='store_false', dest = 'debug', default = True, help='disable compiler debug information')
	opt.add_option('--no-opt', action='store_false', dest = 'opt', default = True, help='disable compiler optimizations')
	opt.add_option('--data-path', action='store', dest = 'data_path', help='the path to install the data to')

def configure(conf):
	if not Options.options.gl and not Options.options.glesv2:
		conf.fatal("You must configure using at least one of --enable-gl, --enable-glesv2")

	conf.check_tool('compiler_cc')
	conf.check_tool('compiler_cxx')

	# Check required headers
	req_headers = ['stdlib.h', 'string.h', 'unistd.h', 'fcntl.h']
	for header in req_headers:
		conf.check_cxx(header_name = header, mandatory = True)
		
	# Check for required libs
	req_libs = [('m', 'm')]
	for (lib, uselib) in req_libs:
		conf.check_cxx(lib = lib, uselib_store = uselib)

	# Check required functions
	req_funcs = [('memset', 'string.h', []) ,('sqrt', 'math.h', ['m'])]
	for func, header, uselib in req_funcs:
		conf.check_cxx(function_name = func, header_name = header, uselib = uselib, mandatory = True)
		
	# Check required packages
	req_pkgs = [('sdl', 'sdl')]
	for (pkg, uselib) in req_pkgs:
		conf.check_cfg(package = pkg, uselib_store = uselib, args = '--cflags --libs',
				mandatory = True)
				
	# Check optional packages
	opt_pkgs = [('gl', 'gl', Options.options.gl), ('egl', 'egl', Options.options.glesv2),
			('glesv2', 'glesv2', Options.options.glesv2)]
	for (pkg, uselib, mandatory) in opt_pkgs:
		conf.check_cfg(package = pkg, uselib_store = uselib, args = '--cflags --libs',
				mandatory = mandatory)

	conf.env.append_unique('CXXFLAGS', '-Wall -Wextra -fms-extensions'.split(' '))

	# Prepend -O# and -g flags so that they can be overriden by the CFLAGS environment variable
	if Options.options.opt:
		conf.env.prepend_value('CXXFLAGS', '-O2')
	if Options.options.debug:
		conf.env.prepend_value('CXXFLAGS', '-g')

	if Options.options.data_path is None:
		Options.options.data_path = os.path.join(conf.env.PREFIX, 'share/glmark2')

	conf.env.append_unique('GLMARK_DATA_PATH', Options.options.data_path)
	conf.env.append_unique('CXXDEFINES', 'GLMARK_DATA_PATH="%s"' % Options.options.data_path)
	conf.env.append_unique('CXXDEFINES', 'GLMARK_VERSION="%s"' % VERSION)

	conf.env.USE_GL = Options.options.gl
	conf.env.USE_GLESv2 = Options.options.glesv2

	conf_message(conf, "Prefix", conf.env.PREFIX)
	conf_message(conf, "Data path", Options.options.data_path)
	conf_message(conf, "Building GL2 version", "Yes" if conf.env.USE_GL else "No")
	conf_message(conf, "Building GLESv2 version", "Yes" if conf.env.USE_GLESv2 else "No")

def build(bld):
	bld.recurse('src')
	bld.recurse('data')
	bld.recurse('doc')
