/*
 * Stand-in for libmysofa/src/hrtf/mysofa_export.h.
 *
 * CMake generates that header via generate_export_header(). Since the externals
 * compile libmysofa's sources directly into themselves rather than linking a
 * shared library, no visibility attributes are needed and MYSOFA_EXPORT expands
 * to nothing -- which is exactly what libmysofa's own fallback template
 * (libmysofa/src/hrtf/mysofa_export.h.in) does for this case.
 *
 * This directory is what gets put on HEADER_SEARCH_PATHS; the config.h next to
 * it resolves reader.c's `#include "../config.h"`.
 */

#ifndef MYSOFA_EXPORT_H
#define MYSOFA_EXPORT_H

#define MYSOFA_EXPORT

#endif
