Install python3.11
Create a python 3.11 virtual environment
Update pip in the new virtual environment
Open a console with activated python 3.11 virtual environment
Install the blender 4.1 module  pip install ~/Downloads/bpy-4.1.0a0-cp311-cp311-manylinux_2_31_x86_64.whl
Install a copy of blender 4.1 built from source provided as blender-t.1
./runblender.sh -P convex_polygon_visibility.py 

Another update on making the polygon visibility volume computation "deployment-ready":  I have created a folder "PolygonVisibilityVolumes" on google drive to facilitate running the polygon visibility algorithm. The algorithm isn't quite there but required subsidiary components are uploaded.

In the final analysis, due to time available, the polygon visibility volume application will manifest as a python script that uses the original custom program to compute visibility volumes (OpenGLDepthRenderer) and the blender 4.1 python scripting module (bpy-4.1.0a0-cp311-cp311-manylinux_2_31_x86_64.whl) installed to a Python 3.11 environment to compute boolean results.

As a by-product of creating the blender 4.1 python module I also make available blender 4.1 application in a compressed file (blender-4.1.tar.gz) which, if desired, can be used to visualize the algorithm processing stages and is also quite possibly useful for capturing images for inclusion in publications. I have used and continue to use the blender 4.1 executable for debugging the algorithm in development.

Meeting the Python 3.11 requirement to execute the blender 4.1 python module is not complicated if running Ubuntu 22.04 or similar up-to-date Linux distribution. Otherwise one will need to download and compile Python 3.11 from source and ensure the ability to run virtual environments from that compiled source which is more involved.

Installation of the python scripting module (bpy-4.1.0a0-cp311-cp311-manylinux_2_31_x86_64.whl) is done via 

"pip install bpy-4.1.0a0-cp311-cp311-manylinux_2_31_x86_64.whl"

From a console with some python 3.11 virtual environment activated.

A couple of notes.

  -  I have been running the python code from the python folder.
  -  Due to a unexpected interface issue you cannot move the mouse during the calculation as it will move the scene and cause the calculation to occur with the wrong pose. This is something fixable but, for now, please do not move your mouse during the program execution.
