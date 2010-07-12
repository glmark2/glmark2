import commands
import subprocess
import os
import Options
import Scripting

blddir = 'build'

top = '.'
VERSION = '0.1.0'
APPNAME = 'glmark2'

# Produce '.tar.gz' with ./waf dist
Scripting.g_gz = 'gz'

def set_options(opt):
	opt.tool_options('compiler_cxx')

	opt.add_option('--no-debug', action='store_false', dest = 'debug', default = True, help='disable compiler debug information')
	opt.add_option('--no-opt', action='store_false', dest = 'opt', default = True, help='disable compiler optimizations')
	opt.add_option('--data-path', action='store', dest = 'data_path', help='the path to install the data to')

def configure(conf):
	conf.check_tool('compiler_cxx')
	conf.check_tool('misc')
	
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
	req_pkgs = [('sdl', 'sdl'), ('gl', 'gl')]
	for (pkg, uselib) in req_pkgs:
		conf.check_cfg(package = pkg, uselib_store = uselib, args = '--cflags --libs',
				mandatory = True)
				
	conf.env.append_unique('CXXFLAGS', '-Wall -Wextra -pedantic'.split(' '))

	# Prepend -O# and -g flags so that they can be overriden by the CFLAGS environment variable
	if Options.options.opt:
		conf.env.prepend_value('CXXFLAGS', '-O2')
	if Options.options.debug:
		conf.env.prepend_value('CXXFLAGS', '-g')

	if Options.options.data_path is None:
		Options.options.data_path = os.path.join(conf.env.PREFIX, 'share/glmark2')

	conf.env.append_unique('GLMARK_DATA_PATH', Options.options.data_path)
	conf.env.append_unique('CXXDEFINES', 'GLMARK_DATA_PATH="%s"' % Options.options.data_path)

	print("Data path: %s" % Options.options.data_path)
	print("Prefix   : %s" % conf.env.PREFIX)

def build(bld):
	bld.recurse('src')
	bld.recurse('data')
