libgoatvr - a modular virtual reality abstraction library
=========================================================

Overview
--------
GoatVR is a simplified abstraction library for dealing with VR headsets. It
is designed to expose a single common API to access VR HMDs through any of the
supported backend modules, such as the Oculus SDK or OpenHMD, even allowing the
application to switch between them at runtime.

Currently implemented VR modules:
 - `oculus`: Oculus SDK
 - `oculus_old`: Oculus SDK 0.5 (last cross-platform version)
 - `openvr`: Valve OpenVR
 - `sbs`: Side-by-Side stereo
 - `anaglyph`: Anaglyph (red-cyan) stereo
 - `stereo`: Quad-buffer stereo

Other modules:
 - `spaceball`: 6dof input source (uses libspnav)

Code examples can be found under the `examples` directory.

Git repo: https://github.com/jtsiomb/libgoatvr.git

License
-------
Copyright (C) 2014-2016 John Tsiombikas <nuclear@member.fsf.org>
GoatVR is free software. You may use, modify, and redistribute this library
under the terms of the GNU Lesser General Public License version 3, or at your
option, any later version published by the Free Software Foundation. See COPYING
and COPYING.LESSER for more details.

Build
-----
Before building libgoatvr you first need to build and install the `gph-math`
library, which can be obtained from: https://github.com/jtsiomb/gph-math

The first time you build libgoatvr, and also every time you add a new module to
the library, you need to run the configure script (or batch file on windows) in
the root directory of the project.

libgoatvr uses cmake to generate build files for any system and compiler. To
generate a makefile and build libgoatvr on UNIX systems, try something like the
following:

    mkdir build        # create out-of-source build directory
    cd build
    cmake ..           # generate makefile
    make               # build
    sudo make install  # to install it

On windows try running the graphical cmake program, then press configure and
generate. This will most likely create a visual studio project, which can be
opened and compiled, linked and optionally installed from within visual studio.

In either case, when running cmake you can choose a number of options, including
which of the optional modules you want to build. Try running `cmake-gui` or
`ccmake` instead of `cmake` if you wish to see and change these options
interactively. Otherwise just pass `-DOPTION=value` arguemnts to cmake; for
instance: `cmake .. -DCMAKE_BUILD_TYPE=Release -Dmod_oculus_old=OFF`
