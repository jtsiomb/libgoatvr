Environment variables
=====================

A list of environment variables accepted by goatvr and its modules.

GoatVR variables
----------------
 - GOATVR_MODULE selects which rendering module to use, overriding the default
   priority-based module selection system.

Module oculus_old
-----------------
 - GOATVR_FAKEHMD enables the fake debug HMD device (`ovrHmd_CreateDebug`).
   This variable can be set to anything to enable the fake HMD, but the
   following special values are recongized and used to select specific models:
   `DK1`, `DK2`, `DKHD`, `BlackStar`, `CB`.
