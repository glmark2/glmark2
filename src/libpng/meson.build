libpng_local_sources = [
    'png.c',
    'pngerror.c',
    'pnggccrd.c',
    'pngget.c',
    'pngmem.c',
    'pngpread.c',
    'pngread.c',
    'pngrio.c',
    'pngrtran.c',
    'pngrutil.c',
    'pngset.c',
    'pngtrans.c',
    'pngvcrd.c',
    'pngwio.c',
    'pngwrite.c',
    'pngwtran.c',
    'pngwutil.c',
]

libpng_local_lib = static_library(
    'libpng-local',
    libpng_local_sources,
    dependencies: zlib_local_dep
    )

libpng_local_dep = declare_dependency(
    link_with: libpng_local_lib,
    include_directories: include_directories('.'),
    dependencies: zlib_local_dep
    )

