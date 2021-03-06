# Virtual Orrery

Uses C++ code that is rendered using OpenGL and the GLFW framework.
Creates an solar system animation (that consists of the sun, earth, and moon) that moves in real time once the program is run. Each object rotates on its own axis and the moon orbits the earth while the earth orbits the sun. The tilts of the orbits and axis rotations are accurate to current available data from NASA.
This program utilized texture mapping, model and view transformations, reference frames, (including spherical camera movement) and real-time animation, all within the viewing pipeline.

## Usage and Keyboard Controls

### Animation Controls:
-------------
0 Key - Resume/Continue

1 Key - Pause 

2 Key - Restart

### Camera Controls:
-------------
Right Mouse Button Click and Drag - Moves camera view in 3D space

Mouse Scroll Button - Zooms camera view in an out

## Running and Compiling
Running and Compliing of this program will require several third party GLFW libraries (argh- 1.3.1, fmt-7.0.3, glew-2.1.0, glfw-3.3.2, glm-0.9.9.7, imgui-1.78, stb-2.26, vivd-2.2.1)

![A snapshot of the animation](virtual_orrery_snapshot.jpg)
