# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `pure-data` submodule, pinned to release `0.52-2`, supplying `m_pd.h`. The
  Xcode project referenced that header as a file next to the `.xcodeproj` but it
  was never in the repository, so no target could compile.

  `0.52-2` matches the Pd that RWA Creator and RWA Player embed via libpd —
  commit `18c9695` in both — since this repository is a submodule of each. The
  externals are built against the oldest Pd they have to load into; they load
  into newer ones fine (verified against Pd 0.56-5). Bumping the pin should be
  tested against those two projects.
- `libmysofa_generated/`, holding the two headers libmysofa's CMake would
  normally generate (`config.h`, `hrtf/mysofa_export.h`). The example projects
  compile libmysofa from source instead of configuring it, so nothing ever
  generated them.
- `PD_INCLUDE_DIR` and `LIBMYSOFA_GENERATED_DIR` build settings, so the Pd
  headers can be pointed at an installed Pd
  (`xcodebuild … PD_INCLUDE_DIR=/Applications/Pd-0.56-5.app/Contents/Resources/src`)
  rather than the submodule.
- README section on building the Pure Data externals on macOS: submodule
  checkout, the `xcodebuild -alltargets` invocation, why `-target` has to be
  used instead of `-scheme` (eight targets, four schemes, one of which does not
  name a target), where the products land, where the three non-repository
  headers come from, the header-map rule that decides whether a file from
  `source/` can be included at all, and how to load-test an external.
- This changelog.
- `.gitignore` entry for `examples/PureData/vas_pd_osx/build`, the Xcode build
  directory. The externals themselves live in `examples/PureData/build`, which
  stays tracked. (Claimed in an earlier revision of this changelog but never
  actually added.)

### Removed

- The built `.pd_darwin` externals are no longer tracked. Nine in
  `examples/PureData/build` and one in
  `examples/PureData/vas_pd_linux/vas_dynconv~` were checked in; they are build
  output and are now ignored via `*.pd_darwin`. `examples/PureData/build` stays
  tracked for the help patches, so the externals have to be built before use.
  One of the nine, `vas_delay~.pd_darwin`, had no matching Xcode target at all —
  it was left over from a target that no longer exists and could not have been
  rebuilt.

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
  deployment target of macOS 11.0 and leaves `x86_64` at 10.13, so Intel
  support is unaffected.

### Fixed

- Engines now deregister from the shared `IRs` filter cache when they are freed.
  The file-based read path (`.txt` HRTF, `.sofa`) registers the loading engine
  in the global `vas_fir_list IRs` so later instances can share the filter, but
  no free routine ever removed the node: after the loading object was deleted
  (patch closed), the list kept a node pointing at the freed engine, and the
  next lookup (`vas_fir_list_find1`, a `strcmp` on the freed engine's
  `fullPath`) touched freed memory. Hosts could only protect themselves by
  wiping the whole cache (RWA Creator called `vas_fir_list_clear()` on every
  simulation stop, which also threw away the pooled patchers' HRTF entry and
  forced a full re-parse of the filter file on the next run).
  `vas_fir_binaural_free()` (the engine free behind every Pd external here) now
  removes its node by pointer match; a no-op for engines that never registered
  (array-loaded IRs, sharing instances). The filter data itself is unaffected:
  it is reference-counted per channel and freed by the last user, exactly as
  before.

- `vas_fir_list_removeNode()` and `vas_fir_list_removeNode1()` dropped the whole
  rest of the list when removing the first node of a multi-node list
  (`firstElement` was set to `NULL` instead of `current->next`), leaking every
  node behind it and forgetting the cached filters. Latent until now (the
  Creator only ever cleared the whole list, and re-reads that hit `removeNode()`
  always re-added their node) but fatal once every engine free removes its own
  node.

- `vas_reverb~`, `vas_partconv~` and `vas_dynconv~` no longer free the Pd arrays
  they read IRs from, which double-freed the buffers and corrupted the heap when
  the patch was closed. `vas_pdmaxobject_set1()` (the `set` method's array path)
  stores the pointer obtained from `garray_getfloatwords()` in
  `x->leftArray`/`x->rightArray`: that is the garray's own live data buffer,
  owned by the patch, only borrowed while the samples are copied into the
  engine's filter. The three externals' free routines called `vas_mem_free()`
  (plain `free()`) on those pointers, so closing the canvas freed each buffer
  twice: once in the external's free, once in `garray_free → array_free →
  freebytes(a_vec)`. Observed as a malloc free-list trap (`SIGTRAP` in
  `free_medium`) in RWA Creator when stopping a simulation whose game contained
  a `vas_reverb~` patch with array-loaded IRs (`libpd_closefile` teardown); in
  plain Pd, deleting the object or closing the patch corrupts the heap the same
  way. A garray resize between `set` and the free (e.g. `soundfiler` reloading)
  makes the stored pointer stale and corrupts differently but just as surely.
  The free routines now leave the array pointers alone; `rwa_binauralsimple~`
  never had the bug.

- All eight Pure Data externals in `examples/PureData/vas_pd_osx` now build from
  a fresh clone **and load into Pd**. An earlier revision of this changelog
  claimed the build was fixed; it was not. The two items below were real but
  only cleared the first errors, and four further problems were left:
  - Every target failed to compile: `m_pd.h`, `mysofa_export.h` and libmysofa's
    `config.h` were all absent from the repository. They are now supplied by the
    `pure-data` submodule and `libmysofa_generated/`, both on
    `HEADER_SEARCH_PATHS` in all sixteen target configurations (target-level
    settings replace the project-level list rather than extending it, so each
    one needs the entries).
  - `rwa_binauralsimple~` was the only target that did not compile
    `source/vas_fir_read.c`, although `vas_pdmaxobject.c` — which every target
    compiles — calls `vas_fir_read_impulseFromFile`. Added to its Compile
    Sources phase.
  - `vas_del~` defined `VAS_USE_LIBMYSOFA` without compiling any libmysofa
    source, so the SOFA reader in `vas_fir.c` was compiled in and left
    `mysofa_open`/`mysofa_close`/`mysofa_getfilter_float` unresolved. `vas_del~`
    is a plain delay object with no SOFA code, so the define is dropped, which
    is what `vas_reverb~` and `vas_partconv~` already do.

    Both of these got through the build because the externals link with
    `-undefined dynamic_lookup`: a missing source file is not a link error, it
    is a `dlopen` failure when Pd loads the object. `rwa_binauralsimple~` and
    `vas_del~` built cleanly and then failed with `symbol not found in flat
    namespace`. All eight are now load-tested, and the README documents how.
  - `MACOSX_DEPLOYMENT_TARGET` was 10.10 in two configurations and 10.12 in
    four more, both below the 10.13 minimum current Xcode accepts; all raised to
    10.13, which silences the warning Xcode emitted on every build.
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
