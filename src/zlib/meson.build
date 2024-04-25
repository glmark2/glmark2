zlib_local_sources = [
    'adler32.c',
    'compress.c',
    'crc32.c',
    'deflate.c',
    'gzclose.c',
    'gzlib.c',
    'gzread.c',
    'gzwrite.c',
    'infback.c',
    'inffast.c',
    'inflate.c',
    'inftrees.c',
    'trees.c',
    'uncompr.c',
    'zutil.c',
]

zlib_local_lib = static_library(
    'zlib-local',
    zlib_local_sources
    )

zlib_local_dep = declare_dependency(
    link_with: zlib_local_lib,
    include_directories: include_directories('.')
    )

