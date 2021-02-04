common_sources = [
    'benchmark-collection.cpp',
    'benchmark.cpp',
    'canvas-generic.cpp',
    'gl-headers.cpp',
    'gl-visual-config.cpp',
    'image-reader.cpp',
    'libmatrix/log.cc',
    'libmatrix/mat.cc',
    'libmatrix/program.cc',
    'libmatrix/shader-source.cc',
    'libmatrix/util.cc',
    'main-loop.cpp',
    'mesh.cpp',
    'model.cpp',
    'options.cpp',
    'scene-buffer.cpp',
    'scene-build.cpp',
    'scene-bump.cpp',
    'scene-clear.cpp',
    'scene-conditionals.cpp',
    'scene.cpp',
    'scene-default-options.cpp',
    'scene-desktop.cpp',
    'scene-effect-2d.cpp',
    'scene-function.cpp',
    'scene-grid.cpp',
    'scene-ideas/a.cc',
    'scene-ideas.cpp',
    'scene-ideas/d.cc',
    'scene-ideas/e.cc',
    'scene-ideas/i.cc',
    'scene-ideas/lamp.cc',
    'scene-ideas/logo.cc',
    'scene-ideas/m.cc',
    'scene-ideas/n.cc',
    'scene-ideas/o.cc',
    'scene-ideas/s.cc',
    'scene-ideas/splines.cc',
    'scene-ideas/table.cc',
    'scene-ideas/t.cc',
    'scene-jellyfish.cpp',
    'scene-loop.cpp',
    'scene-pulsar.cpp',
    'scene-refract.cpp',
    'scene-shading.cpp',
    'scene-shadow.cpp',
    'scene-terrain/base-renderer.cpp',
    'scene-terrain/blur-renderer.cpp',
    'scene-terrain/copy-renderer.cpp',
    'scene-terrain.cpp',
    'scene-terrain/luminance-renderer.cpp',
    'scene-terrain/normal-from-height-renderer.cpp',
    'scene-terrain/overlay-renderer.cpp',
    'scene-terrain/renderer-chain.cpp',
    'scene-terrain/simplex-noise-renderer.cpp',
    'scene-terrain/terrain-renderer.cpp',
    'scene-terrain/texture-renderer.cpp',
    'scene-texture.cpp',
    'shared-library.cpp',
    'text-renderer.cpp',
    'texture.cpp'
]

libmatrix_headers_dep = declare_dependency(
    include_directories: include_directories('libmatrix'),
    compile_args: ['-DUSE_EXCEPTIONS'],
)

common_deps = [
    m_dep,
    dl_dep,
    libjpeg_dep,
    libpng_dep,
    libmatrix_headers_dep,
]
    
if need_drm
    native_drm_lib = static_library(
        'native-drm',
        'native-state-drm.cpp',
        dependencies: [libdrm_dep, libudev_dep, gbm_dep, libmatrix_headers_dep],
        )
    native_drm_dep = declare_dependency(
        link_with: native_drm_lib,
        dependencies: libdrm_dep,
        compile_args: ['-DGLMARK2_USE_DRM', '-D__GBM__'],
        )
else
    native_drm_dep = declare_dependency()
endif

if need_wayland
    wayland_scanner = find_program(wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner'))
    wayland_protocols_dir = wayland_protocols_dep.get_pkgconfig_variable('pkgdatadir')

    xdg_shell_xml_path = wayland_protocols_dir + '/stable/xdg-shell/xdg-shell.xml'
    xdg_shell_client_header = custom_target(
        'xdg-shell client-header',
        command: [ wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@' ],
        input: xdg_shell_xml_path,
        output: 'xdg-shell-client-protocol.h',
        )
    xdg_shell_private_code = custom_target(
        'xdg-shell private-code',
        command: [ wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@' ],
        input: xdg_shell_xml_path,
        output: 'xdg-shell-protocol.c',
        )

    native_wayland_lib = static_library(
        'native-wayland',
        'native-state-wayland.cpp',
        xdg_shell_client_header,
        xdg_shell_private_code,
        dependencies: [libmatrix_headers_dep, wayland_client_dep, wayland_cursor_dep, wayland_egl_dep],
        )

    native_wayland_dep = declare_dependency(
        link_with: native_wayland_lib,
        dependencies: [wayland_client_dep, wayland_cursor_dep, wayland_egl_dep],
        compile_args: ['-DGLMARK2_USE_WAYLAND', '-DWL_EGL_PLATFORM'],
        )
else
    native_wayland_dep = declare_dependency()
endif

if need_x11
    native_x11_lib = static_library(
        'native-x11',
        'native-state-x11.cpp',
        dependencies: [x11_dep, libmatrix_headers_dep],
        )

    native_x11_dep = declare_dependency(
        link_with: native_x11_lib,
        compile_args: ['-DGLMARK2_USE_X11'],
        )
else
    native_x11_dep = declare_dependency()
endif

if need_gl
    gl_gl_lib = static_library(
        'gl-gl',
        'glad/src/gl.c',
        include_directories: include_directories('glad/include'),
        )
    gl_gl_dep = declare_dependency(
        link_with: gl_gl_lib,
        include_directories: include_directories('glad/include'),
        compile_args: ['-DGLMARK2_USE_GL'],
        )

    lib_common_gl = static_library(
        'common-gl',
        common_sources,
        dependencies: [gl_gl_dep, common_deps],
        include_directories: [
            include_directories('scene-ideas'),
            include_directories('scene-terrain'),
            ]
        )
    common_gl_dep = declare_dependency(
        link_with: lib_common_gl,
        include_directories: include_directories('libmatrix'),
        )
else
    gl_gl_dep = declare_dependency()
    common_gl_dep = declare_dependency()
endif

if need_glesv2
    gl_glesv2_lib = static_library(
        'gl-glesv2',
        'glad/src/gles2.c',
        include_directories: include_directories('glad/include'),
        )
    gl_glesv2_dep = declare_dependency(
        link_with: gl_glesv2_lib,
        include_directories: include_directories('glad/include'),
        compile_args: ['-DGLMARK2_USE_GLESv2'],
        )

    lib_common_glesv2 = static_library(
        'common-glesv2',
        common_sources,
        dependencies: [gl_glesv2_dep, common_deps],
        include_directories: [
            include_directories('scene-ideas'),
            include_directories('scene-terrain'),
            ]
        )
    common_glesv2_dep = declare_dependency(
        link_with: lib_common_glesv2,
        include_directories: include_directories('libmatrix'),
        )
else
    gl_glesv2_dep = declare_dependency()
    common_glesv2_dep = declare_dependency()
endif

if need_egl
    # gl-state-egl builds depend on both the GL type and the native type. To
    # avoid creating all the required combinations explicitly, make this a
    # source dependency which will be built properly as part of the final
    # glmark2 executable.
    wsi_egl_dep = declare_dependency(
        sources: ['gl-state-egl.cpp', 'glad/src/egl.c'],
        include_directories: [
            include_directories('glad/include'),
            include_directories('libmatrix'),
            ],
        compile_args: ['-DGLMARK2_USE_EGL'],
        )
else
    wsi_egl_dep = declare_dependency()
endif

if need_glx
    # Although there is a single valid flavor for glx (x11-gl) and we could
    # build gl-state-glx here, make this a source dependency for convenience
    # and parity with gl-state-egl.
    wsi_glx_dep = declare_dependency(
        sources: ['gl-state-glx.cpp', 'glad/src/glx.c'],
        include_directories: [
            include_directories('glad/include'),
            include_directories('libmatrix'),
            ],
        compile_args: ['-DGLMARK2_USE_GLX'],
        )
else
    wsi_glx_dep = declare_dependency()
endif

# Flavor info : [ binary, native, gl, wsi ]
flavor_info = {
  'drm-gl' : ['glmark2-drm', native_drm_dep, gl_gl_dep, wsi_egl_dep],
  'drm-glesv2' : ['glmark2-es2-drm', native_drm_dep, gl_glesv2_dep, wsi_egl_dep],
  'wayland-gl' : ['glmark2-wayland', native_wayland_dep, gl_gl_dep, wsi_egl_dep],
  'wayland-glesv2' : ['glmark2-es2-wayland', native_wayland_dep, gl_glesv2_dep, wsi_egl_dep],
  'x11-gl' : ['glmark2', native_x11_dep, gl_gl_dep, wsi_glx_dep],
  'x11-glesv2' : ['glmark2-es2', native_x11_dep, gl_glesv2_dep, wsi_egl_dep],
}

foreach flavor : flavors
    info = flavor_info[flavor]
    binary = info[0]
    native_dep = info[1]
    gl_dep = info[2]
    wsi_dep = info[3]
    if gl_dep == gl_gl_dep
        common_dep = common_gl_dep
    elif gl_dep == gl_glesv2_dep
        common_dep = common_glesv2_dep
    endif
    executable(
        binary,
        ['main.cpp', 'canvas-generic.cpp'],
        dependencies: [common_dep, native_dep, gl_dep, wsi_dep],
        install: true,
    )
endforeach
