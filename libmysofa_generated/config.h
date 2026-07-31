/*
 * Stand-in for libmysofa/src/config.h.
 *
 * libmysofa normally generates that file from libmysofa/src/config.h.in when it
 * is configured with CMake. The example projects in this repository do not
 * configure libmysofa; they compile its sources straight into each external, so
 * nothing ever expands the @VAR@ placeholders and libmysofa/src/config.h never
 * comes into existence.
 *
 * libmysofa/src/hrtf/reader.c is the only file that includes it, as
 * `#include "../config.h"`. A quoted include that misses relative to its own
 * directory falls back to the header search paths, so putting this file one
 * level above a directory that is on the search path (see hrtf/ next to it)
 * satisfies that include without writing into the submodule.
 *
 * The values only feed mysofa_getversion() and the default.sofa lookup path,
 * neither of which the externals use. They are kept in sync with
 * CPACK_PACKAGE_VERSION_* in libmysofa/CMakeLists.txt by hand.
 */

#if !defined _CONFIG_H
#define _CONFIG_H

#define CMAKE_INSTALL_PREFIX "/usr/local"
#define CPACK_PACKAGE_VERSION_MAJOR 1
#define CPACK_PACKAGE_VERSION_MINOR 2
#define CPACK_PACKAGE_VERSION_PATCH 2

#endif
