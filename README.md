libgoatvr - a modular virtual reality abstraction library
=========================================================

Overview
--------
GoatVR is a simplified abstraction library for dealing with VR headsets. It
is designed to expose a single common API to access VR HMDs through any of the
supported backend modules, such as the Oculus SDK or OpenHMD, even allowing the
application to switch between them at runtime.

Currently implemented VR modules:
 - Oculus SDK
 - Oculus SDK 0.5 (last cross-platform version)
 - Valve OpenVR
 - Side-by-Side stereo

Code examples can be found under the `examples` directory.

Git repo: https://github.com/jtsiomb/libgoatvr.git

License
-------
Copyright (C) 2014-2016 John Tsiombikas <nuclear@member.fsf.org>
GoatVR is free software. You may use, modify, and redistribute this library
under the terms of the GNU Lesser General Public License version 3, or at your
option, any later version published by the Free Software Foundation. See COPYING
and COPYING.LESSER for more details.
