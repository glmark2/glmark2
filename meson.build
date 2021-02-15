project(
    'glmark2',
    ['cpp', 'c'],
    default_options : ['cpp_std=c++14'],
    version : '2021.02',
    meson_version: '>=0.47'
    )

cpp = meson.get_compiler('cpp')

full_version = meson.project_version() + get_option('version-suffix')
add_global_arguments('-DGLMARK_VERSION="@0@"'.format(full_version), language : 'cpp')

data_path = get_option('data-path')
if data_path == ''
    data_path = join_paths([get_option('prefix'), get_option('datadir'), 'glmark2'])
endif
add_global_arguments('-DGLMARK_DATA_PATH="@0@"'.format(data_path), language : 'cpp')

extras_path = get_option('extras-path')
if extras_path != ''
    add_global_arguments('-DGLMARK_EXTRAS_PATH="@0@"'.format(extras_path), language : 'cpp')
endif

m_dep = cpp.find_library('m', required : false)
dl_dep = cpp.find_library('dl')
libjpeg_dep = dependency('libjpeg')
libpng_dep = dependency('libpng')

flavors = get_option('flavors')
if flavors.length() == 0
    error('Need at least one flavor with -Dflavors=flavor1[,flavors2]...\n'+
          'See meson_options.txt contents for available flavors')
endif

flavors_str = ', '.join(flavors)
need_x11 = flavors_str.contains('x11-')
need_drm = flavors_str.contains('drm-')
need_wayland = flavors_str.contains('wayland-')
need_gl = flavors_str.contains('-gl')
need_glesv2 = flavors_str.contains('-glesv2')
need_egl = need_drm or need_wayland or need_glesv2
need_glx = flavors.contains('x11-gl')

if need_x11
    x11_dep = dependency('x11')
endif

if need_drm
    libdrm_dep = dependency('libdrm')
    gbm_dep = dependency('gbm')
    libudev_dep = dependency('libudev')
endif

if need_wayland
    wayland_client_dep = dependency('wayland-client')
    wayland_cursor_dep = dependency('wayland-cursor')
    wayland_egl_dep = dependency('wayland-egl')
    wayland_protocols_dep = dependency('wayland-protocols', version : '>= 1.12')
    wayland_scanner_dep = dependency('wayland-scanner', native: true)
endif

subdir('src')
subdir('data')
subdir('doc')

message('Building with flavors: ' + flavors_str)
