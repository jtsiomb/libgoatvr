Initialization
--------------

Before calling any other function you need to initialize the VR library by
calling ``vr_init``. Similarly, ``vr_shutdown`` frees up all resources held by
the library.

Libgoatvr has a number of backend modules for interfacing with VR headsets, and
there might be multiple modules compiled with your version of libgoatvr. If you
don't want to rely on the built-in defaults, you might want to enumerate and
select which module to use.

The following example initializes the library, and prints the list of available
VR modules:

.. code:: c

 #include <stdio.h>
 #include <goatvr.h>
 
 int main(void)
 {
 	int i, nmod;
 
 	if(vr_init() == -1) {
 		fprintf(stderr, "failed to initialize goatvr\n");
 		return 1;
 	}
 
 	nmod = vr_module_count();
 	printf("There are %d modules:\n", nmod);
 	for(i=0; i<nmod; i++) {
 		printf(" %2d - %s\n", i, vr_module_name(i));
 	}
 	vr_shutdown();
 	return 0;
 }
 
Running this on my computer, with the Oculus Rift DK2 connected, and oculusd
running, produces this output::

 initialized LibOVR 0.4.4
 1 Oculus HMD(s) found
  [0]: Oculus VR - Oculus Rift DK2
 using vr module: libovr
 There are 2 modules:
   0 - libovr
   1 - null

The first four lines come from the initialization of the ``libovr`` module which
is selected by default if it's available. If however I turn off the DK2 or stop
the daemon, then I'm getting just the ``null`` module in this list.

If you'd like to override the default selected module, and use the ``null``
module instead of ``libovr``, which is useful during development to avoid having
to put the HMD on and off all the time, you can call ``vr_use_module(1)``, or
``vr_use_module_named("null")``.
