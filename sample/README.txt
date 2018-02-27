This sample application simply displays a lit, textured model. The view can be
rotated by holding the left mouse button and dragging the cursor.

The EGL and GLESv2 dlls must be in your library path for the sample to run.
This can be accomplished either by placing the dlls into your lib paths or
executing the sample through the provided runsample batch/shell scripts.

In order for the sample to load the model and texture data files, it must be
executed with the current working directory set to the sample's bin folder.

By default, the sample is configured to compile against the 32 bit version of
the libs. If you are running a 64 bit environment, the lib paths in the vcproj
and makefile should be changed to point to the x86-64 folder. Likewise, if you
execute the provided batch/shell scripts on a 64 bit environment, the paths in
these files should be changed to the x86-64 folder.
