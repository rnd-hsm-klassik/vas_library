# vas_library
A library for (dynamic) binaural (room) synthesis (and related fir filtering stuff)

Virtual Acoustic Space (VAS) is a C library for binaural 3D sound reproduction over headphones. In addition to standard objects for Head Related Transfer Function (HRTF)-based dynamic binaural synthesis, delays for simulating reflections, and convolution-based filters for headphone equalization/reverberation, the VAS Library implements a data reduction for binaural room impulse responses (BRIRs).

The VAS library implements a classic, equal partitioned fast convolution algorithm. For the necessary dynamic filter change in realtime, at the moment of the angle change between source and listener, both the impulse response of the old and the target angle are calculated and a crossfade is performed in the time domain. Depending on the audio material, this crossfade must be between 256 and 1024 samples at 44.1 kHz; with 1024 samples, playback is (in my experience) always free of artifacts. 
By splitting IRs in two or more parts and calculting later parts with a greater partition size, a more CPU friendly non-equal partition approach is also realised. This is already done
for the Unity Plugin, a Pure Data version will follow soon.

The raytracing/mirror source plugin is currently rather a proof of concept than a finished plugin. Material characteristics, Air arbsorption etc. are implemented in a very simple manner and
far from physically accurate. Also the number of reflections is rather limited

The male speech audio file used in most examples is from:
http://dx.doi.org/10.14279/depositonce-8536

## Building

## Building the Pure Data externals on macOS

The externals compile straight out of the Xcode project — no CMake step and no
prebuilt libraries. libmysofa, kissfft, pffft and C-Thread-Pool are all compiled
from source into each external, so the submodules have to be checked out first:

```bash
git submodule update --init --recursive
```

Then build every external at once:

```bash
xcodebuild -project examples/PureData/vas_pd_osx/vas_pd_osx.xcodeproj -alltargets -configuration Release build
```

Or a single one, e.g.:

```bash
xcodebuild -project examples/PureData/vas_pd_osx/vas_pd_osx.xcodeproj -target 'vas_binaural~' -configuration Release build
```

Use `-target`, not `-scheme`: the project has eight targets but only four
schemes, and one of them (`rwa_binauralspace~`) does not match any target name.
The full target list is

    rwa_binauralsimple~  vas_binaural~  vas_binauralspace~  vas_partconv~
    vas_dynconv~         vas_reverb~    vas_hpcomp~         vas_del~

`xcodebuild -list -project examples/PureData/vas_pd_osx/vas_pd_osx.xcodeproj`
prints it as well.

Each target links `.pd_darwin` into `examples/PureData/vas_pd_osx/build/Release`
and its Copy Files phase then copies it next to the help patches in
`examples/PureData/build`. The `.pd_darwin` files are build output and are not
tracked. That folder is checked in for the help patches only, so run the build
above before using the externals. To use one, put its `.pd_darwin` either beside
the patch that loads it or into a directory on Pd's search path.

All targets build universal (`arm64` + `x86_64`), so the same binary works on
Apple Silicon and Intel — including under a Rosetta Pd. Deployment target is
macOS 11.0 for the arm64 slice (the minimum Apple Silicon supports) and
10.10/10.12 for x86_64.

The C sources under `source/` are not on any header search path. Xcode resolves
their `#include "…"` through the header map it generates from target
membership, so a file from `source/` that a new external needs must be added to
the Xcode project — the `.h` to the target's Copy Headers phase and the `.c` to
its Compile Sources phase. Adding the directory to `HEADER_SEARCH_PATHS`
instead would diverge from every existing target.

Note that a few submodules are still declared with `git@github.com:` URLs in
`.gitmodules`, which needs an SSH key on GitHub. Without one, rewrite those to
`https://github.com/…` before running the submodule update.

For Linux and Windows use the pd-lib-builder makefiles in
`examples/PureData/vas_pd_linux` instead.

## Usage

Documentation is not complete, most work has been done for the Unity and Pure Data examples, Max/MSP examples are broken right now (soon to be fixed).
Helpfiles for the Pure Data objects vas_binaural~, vas_reverb~ and vas_hpcomp~ are in Examples/PureData/doc

In the Examples folder are implementations for dynamic binaural (room) synthesis for Pure Data and Unity. Pd examples are XCode Projects for OSX. 
The crossplatform makefile in Examples/PureData/vas_pd_linux should work for linux, osx and windows (only tested for linux).
For Unity you will find both, Visual Studio and XCode Projects. 

In Pure Data, the HRTF/BRIR should be placed in the same folder where the Pd Patch resides. Then you can simply pass the HRTF/BRIR name as the first argument then followed by partition size as second argument and an offset for the IR. Useful partition sizes are 512 or 1024 samples for BRIR Synthesis and up to 64 Samples for HRTF Synthesis.
For the Unity Plugin on, put the BRIR/HRTF files inside the 'StreamingAssets' Folder. 

A sofa to vas textformat converter is in the matlab folder. Sofa support will come back soon

## To do next

Include sofa support again.
Redo MaxMSP objects.
Material Characteristics for the Unity spatializer.
Change fft framework for non-Apple OSs.
