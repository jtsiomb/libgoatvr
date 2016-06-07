Initialization
--------------

Before calling any other function you need to initialize the VR library by
calling ``goatvr_init``. Similarly, ``goatvr_shutdown`` frees up all resources
held by the library.

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
 
 	if(goatvr_init() == -1) {
 		fprintf(stderr, "failed to initialize goatvr\n");
 		return 1;
 	}
 
 	nmod = goatvr_num_modules();
 	printf("There are %d modules:\n", nmod);
 	for(i=0; i<nmod; i++) {
                goatvr_module *mod = goatvr_get_module(i);
 		printf(" %2d - %s\n", i, goatvr_module_name(mod));
 	}
 	goatvr_shutdown();
 	return 0;
 }

If you'd like to override the default module selection, you can call
``goatvr_activate_module(mod)``, where ``mod`` is a module pointer returned by
``goatvr_get_module`` (by ordinal) or ``goatvr_find_module`` (by name).

An alternative way to select the module to be used by goatvr, is to set the
``GOATVR_MODULE`` environment variable. For instance, to select the *OpenVR*
module, do the following: ``GOATVR_MODULE=openvr; export GOATVR_MODULE``
(UNIX, bourne shell), ``setenv GOATVR_MODULE openvr`` (UNIX, c-shell), or
``set GOATVR_MODULE=openvr`` (windows, cmd).

To enter VR mode, you must call ``goatvr_startvr``.
