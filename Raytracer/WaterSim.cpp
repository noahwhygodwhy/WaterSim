#include "WaterSim.hpp"
#include "clTypeDefs.hpp"

typedef float4 fvec4;
typedef float3 fvec3;

#define CPP
//#include "sharedStructs.cl"

using namespace std;
using namespace std::filesystem;
using namespace glm;




constexpr uint32_t bathX = 400;
constexpr uint32_t bathY = 400;

uint32_t frameX = 400;
uint32_t frameY = 400;
double frameRatio = double(frameX) / double(frameY);


fvec4 clearColor(0.21, 0.78, 0.95, 1.0);

//fvec4 clearColor(0.0, 0.0, 0.0, 1.0);

double deltaTime = 0.0f;	// Time between current frame and last frame
double lastFrame = 0.0f; // Time of last frame
string saveFileDirectory = "";


constexpr double bias = 1e-4;


void frameBufferSizeCallback(GLFWwindow* window, uint64_t width, uint64_t height) {
	glViewport(0, 0, GLsizei(width), GLsizei(height));
}

void saveImage(string filepath, GLFWwindow* w) {

	string outDir = "out/"+saveFileDirectory+"/";
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 4;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<float> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png((outDir + filepath).c_str(), width, height, nrChannels, buffer.data(), stride);
}
void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}
//
//void initOpenGL(GLFWwindow* window) {
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	glfwWindowHint(GLFW_SAMPLES, 16);
//	window = glfwCreateWindow(GLsizei(frameX), GLsizei(frameY), "Renderer", NULL, NULL);
//	if (window == NULL)
//	{
//		cout << "Window creation failed" << endl;
//		exit(-1);
//	}
//	glfwMakeContextCurrent(window);
//
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//	{
//		cout << "GLAD init failed" << endl;
//		exit(-1);
//	}
//	frameBufferSizeCallback(window, frameX, frameY);
//}

void initOpenCL(cl_context& clContext, cl_device_id& device, GLFWwindow* window) {

	cl_uint numPlatforms;
	cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clGetPlatformIDs failed (1)\n");
	//printf("Number of Platforms = %d\n", numPlatforms);
	cl_platform_id* platforms = new cl_platform_id[numPlatforms];
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clGetPlatformIDs failed (2)\n");
	cl_uint numDevices;
	cl_device_id* devices;

	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	devices = new cl_device_id[numDevices];
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	cl_device_type type;
	size_t sizes[3] = { 0, 0, 0 };
	clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, sizeof(type), &type, NULL);
	device = devices[0];

	printf("device type: %ui\n", type);



	char* ui;
	//size_t valueSize;
	//clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(ui), &ui, NULL);
	//printf("%s\n", ui);
	//clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, NULL, &valueSize);
	//printf("%s\n", ui);
	ui = (char*)malloc(1024);
	clGetDeviceInfo(device, CL_DEVICE_VERSION, 1024, ui, NULL);
	printf("%s\n", ui);
	clGetDeviceInfo(device, CL_DEVICE_VENDOR, 1024, ui, NULL);
	printf("%s\n", ui);
	clGetDeviceInfo(device, CL_DRIVER_VERSION, 1024, ui, NULL);
	printf("%s\n", ui);
	clGetDeviceInfo(device, CL_DEVICE_PROFILE, 1024, ui, NULL);
	printf("%s\n", ui);
	clGetDeviceInfo(device, CL_DEVICE_PLATFORM, 1024, ui, NULL);
	printf("%i\n", ui);
	clGetDeviceInfo(device, CL_DEVICE_NAME, 1024, ui, NULL);
	printf("%s\n", ui);


	printf(" %d.%d Hardware version: %s\n", 1, 1, ui);
	free(ui);

	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetWGLContext(window),
		CL_WGL_HDC_KHR, (cl_context_properties)GetDC(glfwGetWin32Window(window)),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0],
	0 };

	clContext = clCreateContext(properties, 1, &device, NULL, NULL, &status);
	printf("opencl context status: %i\n", status);

	
}

int main()
{
	srand(0u);
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 16);
	GLFWwindow* window = glfwCreateWindow(GLsizei(frameX), GLsizei(frameY), "Renderer", NULL, NULL);
	if (window == NULL)
	{
		cout << "Window creation failed" << endl;
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "GLAD init failed" << endl;
		exit(-1);
	}
	frameBufferSizeCallback(window, frameX, frameY);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	cl_int status = 0;
	cl_context clContext;
	cl_device_id device;
	//initOpenGL(window);
	


	initOpenCL(clContext, device, window);





//Set up opencl memory
	printf("context status: %i\n", status);
	cl_command_queue_properties* qProperties = new cl_command_queue_properties();
	cl_command_queue cmdQueue = clCreateCommandQueueWithProperties(clContext, device, qProperties, &status);
	printf("cmdqueue status: %i\n", status);

	cl_mem bathEven = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(uint32_t) * bathX * bathY, NULL, &status);
	printf("create bathEven buffer status: %i\n", status);
	cl_mem bathOdd = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(uint32_t) * bathX * bathY, NULL, &status);
	printf("create bathood buffer status: %i\n", status);
	cl_mem bathTemp = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(float) * bathX * bathY, NULL, &status);
	printf("create bathTemp buffer status: %i\n", status);
	//cl_mem clFrameCount = clCreateBuffer(clContext, CL_MEM_READ_ONLY, sizeof(uint32_t), NULL, &status);
	//printf("create clframecount buffer status: %i\n", status);
	




//Set up opengl buffer to be drawn to by opencl
	GLuint frameFBO;
	glGenFramebuffers(1, &frameFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, frameFBO);

	unsigned int frameTexture;
	glGenTextures(1, &frameTexture);
	glBindTexture(GL_TEXTURE_2D, frameTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, bathX, bathY, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameTexture, 0);

	printf("frame texture: %i\n", frameTexture);
	printf("frameFBO: %i\n", frameFBO);



	cl_mem clFrameTexture = clCreateFromGLTexture(clContext, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, frameTexture, &status);
	printf("create buffer 5 status: %i\n", status);
	status = clEnqueueAcquireGLObjects(cmdQueue, 1, &clFrameTexture, 0, NULL, NULL);
	printf("aquire gl object status: %i\n", status);


	//printf("cltesttexture status: %i\n", status);



//read in opencl program and compile
	long unsigned int length;
	ifstream stream("WaterSim.cl", ios::in | ios::ate | ios::binary);
	//stream.seekg(0, ios::end);
	length = long unsigned int(stream.tellg());
	stream.seekg(0, ios::beg);
	char* shaderSource = new char[length + 1l];
	shaderSource[length] = '\0';
	const char** shaderSourcesArray = new const char* [1];
	stream.read(shaderSource, length);

	shaderSourcesArray[0] = shaderSource;

	cl_program program = clCreateProgramWithSource(clContext, 1, shaderSourcesArray, NULL, &status);

	const char* options = { "-cl-single-precision-constant" };
	status = clBuildProgram(program, 1, &device, options, NULL, NULL);

	if (status != CL_SUCCESS)
	{ // retrieve and print the error messages:
		size_t size;

		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
		cl_char* log = new cl_char[size];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size, log, NULL);
		printf("clBuildProgram failed:\n%s\n", log);
		exit(-1);
	}




//create the kernels and set memory arguments
	cl_kernel waterSimKernel = clCreateKernel(program, "watersim", &status);
	printf("kernal status: %i\n", status);
	cl_kernel renderKernel = clCreateKernel(program, "render", &status);
	printf("kernal status: %i\n", status);



	status = clSetKernelArg(renderKernel, 1, sizeof(cl_mem), &clFrameTexture);
	printf("set arg 1 status: %i\n", status);
	








//Documentation todo:
	//v this is the number of items to do, so like framex and framey?
	//size_t globalWorkSize[3] = { sizes[0], sizes[1], sizes[2]};

	//values in this must be divisible by values in local work size
	//cannot be values larger than an unsigned in contained in a number of bits queried by CL_DEVICE_ADDRESS_BITS
	size_t globalWorkSize[3] = { bathX, bathY, 1 };

	//this is describing how wide the processing can be, so we set it to max the gpu can do
	//size_t localWorkSize[3] = { sizes[0], sizes[1], sizes[2]};


	//must be a 3d array volume < CL_DEVICE_MAX_WORK_GROUP_SIZE (in our case 1024)
	//and each side must be less than the coresponding size in CL_DEVICE_MAX_WORK_ITEM_SIZES (1024x1024x64)
	size_t localWorkSize[3] = { NULL, NULL, NULL };




//opengl shader
	Shader shader("vert.glsl", "frag.glsl");
	shader.use();


//set up initial random buffer state cause gpu random is hard
	/*mt19937_64 numGen;
	randomBuffer = new uint64_t[frameX * frameY]();
	for (size_t i = 0; i < frameX * frameY; i++) {
		randomBuffer[i] = numGen();
	}*/
	



	uint32_t frameCounter = 0;
	float frameTimes[30](0);
	int lastSecondFrameCount = -1;

	//uint32_t fps = 12;

	//cl_event* waitAfterWrites = new cl_event[5];

	
	uint32_t* initialBathState = new uint32_t[bathX * bathY]();


	for (uint64_t i = 0; i < bathX * bathY; i++) {
		if ((i % bathX) < (bathX/2)) {
			initialBathState[i] = 1;
		}
		else {
			initialBathState[i] = 0;
		}
	}

	printf("%i\n", initialBathState[0]);

	//status = clEnqueueWriteBuffer(cmdQueue, bathOdd, CL_TRUE, 0, sizeof(uint32_t)*bathX*bathY, initialBathState, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(cmdQueue, bathEven, CL_TRUE, 0, sizeof(uint32_t) * bathX * bathY, initialBathState, 0, NULL, NULL);
	printf("write 0 status: %i\n", status);



	//cl_event* otherDataEvent = new cl_event;
	//cl_event* waitAfterProcessing = new cl_event;



	while (!glfwWindowShouldClose(window)) {
		frameCounter++;
		double currentFrame = glfwGetTime();
		//frame time and fps calculation
		deltaTime = currentFrame - lastFrame;
		//printf("that frame took %f seconds\n", deltaTime);
		lastFrame = currentFrame;

		if (int(currentFrame) > lastSecondFrameCount) {
			lastSecondFrameCount = int(currentFrame);
			float sum = 0;
			for (float f : frameTimes) {
				sum += f;
			}
			printf("fps: ~%f\n", sum/30.0f);
		}
		frameTimes[frameCounter % 30] = 1.0f / float(deltaTime);





		////set the frame counter argument
		//status = clSetKernelArg(waterSimKernel, 7, sizeof(cl_uint), &frameCounter);
		//printf("set arg 7 status: %i\n", status);
		//
		//status = clEnqueueWriteBuffer(cmdQueue, clToneMapData, CL_TRUE, 0, sizeof(ToneMapStruct), &toneMapData, 0, NULL,NULL);
		//printf("write tonemap status: %i\n", status);

		cl_int fc = frameCounter;
		status = clSetKernelArg(waterSimKernel, 2, sizeof(cl_int), &fc);

		if (frameCounter % 2 == 0) {

			status = clSetKernelArg(waterSimKernel, 0, sizeof(cl_mem), &bathOdd);
			status = clSetKernelArg(waterSimKernel, 1, sizeof(cl_mem), &bathEven);
			status = clSetKernelArg(renderKernel, 0, sizeof(cl_mem), &bathEven);
		}
		else {
			status = clSetKernelArg(waterSimKernel, 0, sizeof(cl_mem), &bathEven);
			status = clSetKernelArg(waterSimKernel, 1, sizeof(cl_mem), &bathOdd);
			status = clSetKernelArg(renderKernel, 0, sizeof(cl_mem), &bathOdd);
		}


		cl_event* firstPassEvent = new cl_event();
		status = clEnqueueNDRangeKernel(cmdQueue, waterSimKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, firstPassEvent);
		if(status != 0)printf("range kernel: %i\n", status);
		clWaitForEvents(1, firstPassEvent);
		cl_event* secondPassEvent = new cl_event();
		status = clEnqueueNDRangeKernel(cmdQueue, renderKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, secondPassEvent);
		if (status != 0)printf("range kernel: %i\n", status);
		clWaitForEvents(1, secondPassEvent);

		clFinish(cmdQueue);

		//draw it to the glfw window
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameFBO);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glBlitFramebuffer(0, 0, bathX, bathY, 0, 0, frameX, frameY, GL_COLOR_BUFFER_BIT, GL_NEAREST);



		glfwSwapBuffers(window);

		processInput(window);
		glfwPollEvents();

		
#ifdef CIN
		cin.get();
#endif

	}
	//pool.stop();
	printf("closing\n");
	
	glfwTerminate();
}


