project(
    'glmark2',
    ['cpp', 'c'],
    default_options : ['cpp_std=c++17'],
    version : '2023.01',
    meson_version: '>=0.47'
    )

cpp = meson.get_compiler('cpp')

full_version = meson.project_version() + get_option('version-suffix')
add_global_arguments('-DGLMARK_VERSION="@0@"'.format(full_version), language : 'cpp')

data_path = get_option('data-path')
if data_path == ''
    data_path = join_paths([get_option('prefix'), get_option('datadir'), 'glmark2'])
endif
add_global_arguments('-DGLMARK_DATA_PATH=R"(@0@)"'.format(data_path), language : 'cpp')

extras_path = get_option('extras-path')
if extras_path != ''
    add_global_arguments('-DGLMARK_EXTRAS_PATH=R"(@0@)"'.format(extras_path), language : 'cpp')
endif

flavors = get_option('flavors')
if flavors.length() == 0
    error('Need at least one flavor with -Dflavors=flavor1[,flavors2]...\n'+
          'See meson_options.txt contents for available flavors')
endif

flavors_str = ', '.join(flavors)
need_x11 = flavors_str.contains('x11-')
need_drm = flavors_str.contains('drm-')
need_gbm = flavors_str.contains('gbm-')
need_wayland = flavors_str.contains('wayland-')
need_null = flavors_str.contains('null-')
need_gl = flavors_str.contains('-gl')
need_glesv2 = flavors_str.contains('-glesv2')
need_egl = need_drm or need_wayland or need_glesv2 or need_null or flavors_str.contains('-egl')
need_glx = flavors.contains('x11-gl')
need_win32 = flavors_str.contains('win32-')
need_wgl = 'win32-gl' in flavors
need_dispmanx = flavors_str.contains('dispmanx-')

if need_win32 and (need_x11 or need_gbm or need_drm or need_wayland or need_dispmanx)
    error('Simultaneous Windows and non-Windows builds are not supported')
endif

if need_x11
    x11_dep = dependency('x11')
endif

if need_drm
    libdrm_dep = dependency('libdrm')
    gbm_dep = dependency('gbm')
    libudev_dep = dependency('libudev')

    # gbm >= 17.1 required for multiplane
    if gbm_dep.version().version_compare('>= 17.1')
        add_global_arguments('-DGBM_HAS_PLANES', language : 'cpp')
    endif
endif

if need_gbm
    gbm_dep = dependency('gbm')
endif

if need_wayland
    wayland_client_dep = dependency('wayland-client')
    wayland_cursor_dep = dependency('wayland-cursor')
    wayland_egl_dep = dependency('wayland-egl')
    wayland_protocols_dep = dependency('wayland-protocols', version : '>= 1.12')
    wayland_scanner_dep = dependency('wayland-scanner', native: true)
endif

if need_dispmanx
    bcm_host_dep = dependency('bcm_host')
endif

if need_win32
    add_global_arguments('-D_USE_MATH_DEFINES', language : 'cpp')
    add_global_arguments('-DWIN32', language : 'cpp')
    opengl32_dep = cpp.find_library('opengl32')
endif

m_dep = cpp.find_library('m', required : false)
dl_dep = cpp.find_library('dl', required : false)
thread_dep = dependency('threads')

if host_machine.system() == 'windows'
    if cpp.get_id() == 'msvc'
        add_global_arguments('/EHsc', '/wd4146', '/wd4244', '/wd4305', '/wd4312',
                             language: 'cpp')
    endif
    subdir('src/zlib')
    subdir('src/libpng')
    subdir('src/libjpeg-turbo')
    libjpeg_dep = libjpeg_turbo_local_dep
    libpng_dep = libpng_local_dep
else
    libjpeg_dep = dependency('libjpeg')
    libpng_dep = dependency('libpng')
endif

subdir('src')
subdir('data')
subdir('doc')

message('Building with flavors: ' + flavors_str)
