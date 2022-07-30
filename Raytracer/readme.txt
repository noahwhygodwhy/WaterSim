Raytracer
Author: Noah Alvard
Build with Visual Studio C++20
Dependencies through nuget
	glfw@3.3.7
	glm@0.9.9.800
	opencl-nug@0.777.77

This is the GPU accelerated version of the path tracer using opencl. 
Unlike the CPU version, it does not have a kd tree, so a lot of shapes slows it down, cause I ran out of time, but it does have volumetric fog,
Although by default it's a lot faster cause GPU, until you start adding like >10 shapes, then it struggles.


How to configure: 
	frameX and frameY are the window size
	the background color is set at the top of Raytracer.cpp
	materials and shapes are set in the main() function by pushing to the material and shapes vectors
		see sharedStructs.cl for what goes into a material
	eye position is set in raytracer.cl
	Outside of these, there are a series of #defines at the top of Raytracer.cpp that change the program's behavior
	MAX_SHAPES and MAX_MATERIALS define the maximum size of the buffers the gpu has. attemps to put more will cause a crash, but then these values can just be changed
	MAX_PATH is the maximum number of bounces a ray will take.
	OUTPUTFRAMES means the program will output images for each frame, the number following output frames defining how many before it stops
    EVERYFRAME, bases the "current time" on the frame, not on actual time passed
	CIN calls cin.get() between frames so that each frame can be seen for troubleshooting purposes
	PIXEL_MULTISAMPLE_N doesn't do anything in this version
	MONTE_CARLO_SAMPLES is the number of samples per pixel
