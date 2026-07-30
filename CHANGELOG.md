# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- README section on building the Pure Data externals on macOS: submodule
  checkout, the `xcodebuild -alltargets` invocation, why `-target` has to be
  used instead of `-scheme` (eight targets, four schemes, one of which does not
  name a target), where the products land, and the header-map rule that decides
  whether a file from `source/` can be included at all.
- This changelog.
- `.gitignore` entry for `examples/PureData/vas_pd_osx/build`, the Xcode build
  directory. The externals themselves live in `examples/PureData/build`, which
  stays tracked.

### Changed

- Resolve IR files via Pd's search path:

  `vas_pdmaxobject_read` concatenated the canvas directory with the filename,
  so an IR was only ever found next to the patch loading it. Use
  `open_via_path`: absolute path, then the patch directory, then the global
  search path a host can extend with `libpd_add_to_search_path`.

  Also bounds the path (fullpath is 512 bytes, `MAXPDSTRING` is 1000) and
  reports a missing file instead of passing a bad path down.
- The Pd externals are now universal binaries (`arm64` + `x86_64`). Every target
  except `vas_partconv~` was pinned to Intel through
  `ARCHS = "$(ARCHS_STANDARD_64_BIT)"` together with
  `VALID_ARCHS = "i386 x86_64"`, the second of which filtered `arm64` out even
  when `ARCHS` asked for it, so the externals could not be loaded by a native
  Apple Silicon Pd. Both settings now match what `vas_partconv~` already used
  (`$(ARCHS_STANDARD)` / `"x86_64 arm64"`). Xcode gives the `arm64` slice a
  deployment target of macOS 11.0 and leaves `x86_64` at 10.10/10.12, so Intel
  support is unaffected. The checked-in binaries in `examples/PureData/build`
  have been rebuilt accordingly.

### Fixed

- All eight Pure Data externals in `examples/PureData/vas_pd_osx` build again
  from a fresh clone. Two independent problems stood between a checkout and a
  working `.pd_darwin`:
  - Five targets (`rwa_binauralsimple~`, `vas_binaural~`, `vas_binauralspace~`,
    `vas_hpcomp~`, `vas_dynconv~`) listed thirteen libmysofa object files —
    `libmysofa/src/hrtf/{cache,check,easy,interpolate,kdtree,lookup,loudness,minphase,neighbors,reader,resample,spherical,tools}.o`
    in their link phase. Those are the leftovers of an in-place build of the
    libmysofa submodule and do not exist in a fresh checkout, so the build
    stopped with `Build input files cannot be found` before compiling anything.
    The same translation units are already in each target's Compile Sources
    phase, so dropping the object files changes nothing about what gets linked.
  - `vas_dynconv~` never compiled at all: `vas_dynconv~.h` includes
    `vas_firobject.h`, but neither `source/vas_firobject.h` nor
    `source/vas_firobject.c` was referenced anywhere in the project. Both are
    now project files, the header in the target's Copy Headers phase and the
    implementation in Compile Sources. No source change was needed — the file
    compiles as it stands.
