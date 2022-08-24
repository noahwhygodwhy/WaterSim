#include "WaterSim.hpp"
#include <glm/gtx/normal.hpp>
#include "KDTree.hpp"

typedef float4 fvec4;
typedef float3 fvec3;

//#define CIN
#define CPP
//#include "sharedStructs.cl"

using namespace std;
using namespace std::filesystem;
using namespace glm;



uint32_t frameX = 1000;
uint32_t frameY = 1000;


vec3 boxSize = vec3(10.0f);
float ballRad = 0.8f;

double frameRatio = double(frameX) / double(frameY);


fvec4 clearColor(0.21, 0.78, 0.95, 1.0);

//fvec4 clearColor(0.0, 0.0, 0.0, 1.0);

double deltaTime = 0.0f;	// Time between current frame and last frame
double lastFrame = 0.0f; // Time of last frame
string saveFileDirectory = "";

constexpr double bias = 1e-4;
constexpr uint32_t MAX_PARTICLES = 2000;
//constexpr uint32_t KD_MAX_LAYERS = 20;


void frameBufferSizeCallback(GLFWwindow* window, uint64_t width, uint64_t height) {
	frameX = width;
	frameY = height;
	glViewport(0, 0, GLsizei(width), GLsizei(height));
}


void GLAPIENTRY MessageCallback(GLenum source,
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

	printf("device type: %lu\n", type);



	char* ui;
	ui = (char*)malloc(1024);


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


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(MessageCallback, 0);
	
	cl_int status = 0;
	cl_context clContext;
	cl_device_id device;
	initOpenCL(clContext, device, window);





//Set up opencl memory
	printf("context status: %i\n", status);
	cl_command_queue_properties* qProperties = new cl_command_queue_properties();
	//*qProperties = CL_QUEUE_PROFILING_ENABLE;

	cl_command_queue cmdQueue = clCreateCommandQueueWithProperties(clContext, device, qProperties, &status);
	printf("cmdqueue status: %i\n", status);


//shared context buffer

	Particle* initialParticles = new Particle[MAX_PARTICLES]();

	uint32_t maxTreeMem = size_t(glm::pow(2, glm::ceil(glm::log2(float(MAX_PARTICLES)) + 1))) - 1;

	//uint32_t maxTreeMem = uint32_t(glm::pow(2, glm::ceil(glm::log2(float(MAX_PARTICLES + 1))))) - 1;
	cl_mem clKDQueueA = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(ivec4) * maxTreeMem, NULL, &status);
	cl_mem clKDQueueB = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(ivec4) * maxTreeMem, NULL, &status);
	cl_mem clTheTree = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(KDNode) * maxTreeMem, NULL, &status);

	cl_mem clTotalList = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(int32_t) * MAX_PARTICLES, NULL, &status);
	cl_mem clNodeIndexList = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(int32_t) * MAX_PARTICLES, NULL, &status);

	uint32_t numberOfPoints = 0;



	ivec3 counts = boxSize / (ballRad * 2);

	//printf("%i, %i, %i\n", counts.x, counts.y, counts.z);

	/*for (int i = 0; i < 1000; i++) {
		float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		printf("%f\n", x);
	}
	exit(1);*/


	for (int i = 0; i < MAX_PARTICLES; i++) {
		float xR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.x;
		float yR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.y;
		float zR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.z;

		initialParticles[numberOfPoints] = Particle(
			fvec4(xR, yR, zR, 0),
			fvec4(0),
			fvec4(0),
			0.0f,
			0.0f
		);

		numberOfPoints++;
		
	}
	printf("numberofballs: %u\n", numberOfPoints);



	GLuint particleVBO, particleVAO;
	glGenVertexArrays(1, &particleVAO);
	glBindVertexArray(particleVAO);
	glGenBuffers(1, &particleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * MAX_PARTICLES, initialParticles, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)offsetof(Particle, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)offsetof(Particle, velocity));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)offsetof(Particle, force));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)offsetof(Particle, density));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)offsetof(Particle, pressure));
	glEnableVertexAttribArray(3);

	cl_mem clParticles = clCreateFromGLBuffer(clContext, CL_MEM_READ_WRITE, particleVBO, &status);
	if (status)printf("positions clmem from VBO %i\n", status);
	status = clEnqueueAcquireGLObjects(cmdQueue, 1, &clParticles, 0, NULL, NULL);
	if (status)printf("aquire gl objects %i\n", status);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);




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
	if (status)printf("cl build program %i\n", status);

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
	cl_kernel bitonicSortKernel = clCreateKernel(program, "bitonicSort", &status);
	if (status)printf("bitonic sort kernel %i\n", status);
	cl_kernel computeDPKernel = clCreateKernel(program, "computeDP", &status);
	if (status)printf("computedp kernel %i\n", status);
	cl_kernel kdTreeKernel = clCreateKernel(program, "makeKDTree", &status);
	if (status)printf("kdtreekernel %i\n", status);



	KDConstructionContext kdConCon(
		initialParticles,
		numberOfPoints,
		&clTotalList,
		&clKDQueueA,
		&clKDQueueB,
		&clTheTree,
		&cmdQueue,
		&kdTreeKernel,
		&bitonicSortKernel,
		&clNodeIndexList
	);



	status = clSetKernelArg(computeDPKernel, 0, sizeof(cl_mem), &clParticles);
	if (status)printf("dp kernel 0 %i\n", status);
	status = clSetKernelArg(computeDPKernel, 1, sizeof(cl_mem), &clTheTree);
	if (status)printf("dp kernel 1 %i\n", status);

	status = clSetKernelArg(*kdConCon.kdTreeKernel, 0, sizeof(cl_mem), &clParticles);
	if (status)printf("kd kernel 0 %i\n", status);
	status = clSetKernelArg(*kdConCon.kdTreeKernel, 2, sizeof(cl_mem), &clTheTree);
	if (status)printf("kd kernel 2 %i\n", status);
	status = clSetKernelArg(*kdConCon.kdTreeKernel, 6, sizeof(cl_mem), &clNodeIndexList);
	if (status)printf("kd kernel 6 %i\n", status);

	status = clSetKernelArg(*kdConCon.bitonicKernel, 0, sizeof(cl_mem), &clParticles);
	if (status)printf("bitonic kernel 0 %i\n", status);
	status = clSetKernelArg(*kdConCon.bitonicKernel, 1, sizeof(cl_mem), &clTotalList);
	if (status)printf("bitonic kernel 0 %i\n", status);
	status = clSetKernelArg(*kdConCon.bitonicKernel, 2, sizeof(cl_mem), &clNodeIndexList);
	if (status)printf("bitonic kernel 0 %i\n", status);
	

	size_t globalWorkSize[3] = { MAX_PARTICLES, 1, 1 };
	size_t localWorkSize[3] = { NULL, NULL, NULL };

	Shader boxShader("boxVert.glsl", "boxFrag.glsl");
	Shader waterShader("waterVert.glsl", "waterFrag.glsl");


	uint32_t frameCounter = 0;
	float frameTimes[30](0);
	int lastSecondFrameCount = -1;



	status = clEnqueueWriteBuffer(cmdQueue, clParticles, CL_TRUE, 0, sizeof(Particle) * MAX_PARTICLES, initialParticles, 0, NULL, NULL);
	if (status)printf("write initial particles %i\n", status);
	
	




	fvec3 finalBoxData[72] = {
		vec3(0.000000, 1.000000, 0.000000), vec3(0.000000, 0.000000, 1.000000),
		vec3(0.000000, 0.000000, 0.000000), vec3(0.000000, 0.000000, 1.000000),
		vec3(1.000000, 0.000000, 0.000000), vec3(0.000000, 0.000000, 1.000000),

		vec3(0.000000, 1.000000, 0.000000), vec3(0.000000, 0.000000, 1.000000),
		vec3(1.000000, 0.000000, 0.000000), vec3(0.000000, 0.000000, 1.000000),
		vec3(1.000000, 1.000000, 0.000000), vec3(0.000000, 0.000000, 1.000000),

		vec3(0.000000, 1.000000, 1.000000), vec3(1.000000, 0.000000, 0.000000),
		vec3(0.000000, 0.000000, 0.000000), vec3(1.000000, 0.000000, 0.000000),
		vec3(0.000000, 1.000000, 0.000000), vec3(1.000000, 0.000000, 0.000000),

		vec3(0.000000, 1.000000, 1.000000), vec3(1.000000, 0.000000, 0.000000),
		vec3(0.000000, 0.000000, 1.000000), vec3(1.000000, 0.000000, 0.000000),
		vec3(0.000000, 0.000000, 0.000000), vec3(1.000000, 0.000000, 0.000000),

		vec3(1.000000, 1.000000, 1.000000), vec3(0.000000, 0.000000, -1.000000),
		vec3(0.000000, 0.000000, 1.000000), vec3(0.000000, 0.000000, -1.000000),
		vec3(0.000000, 1.000000, 1.000000), vec3(0.000000, 0.000000, -1.000000),

		vec3(1.000000, 1.000000, 1.000000), vec3(0.000000, 0.000000, -1.000000),
		vec3(1.000000, 0.000000, 1.000000), vec3(0.000000, 0.000000, -1.000000),
		vec3(0.000000, 0.000000, 1.000000), vec3(0.000000, 0.000000, -1.000000),

		vec3(1.000000, 1.000000, 0.000000), vec3(-1.000000, 0.000000, 0.000000),
		vec3(1.000000, 0.000000, 1.000000), vec3(-1.000000, 0.000000, 0.000000),
		vec3(1.000000, 1.000000, 1.000000), vec3(-1.000000, 0.000000, 0.000000),

		vec3(1.000000, 1.000000, 0.000000), vec3(-1.000000, 0.000000, 0.000000),
		vec3(1.000000, 0.000000, 0.000000), vec3(-1.000000, 0.000000, 0.000000),
		vec3(1.000000, 0.000000, 1.000000), vec3(-1.000000, 0.000000, 0.000000),

		vec3(0.000000, 0.000000, 1.000000), vec3(0.000000, 1.000000, 0.000000),
		vec3(1.000000, 0.000000, 0.000000), vec3(0.000000, 1.000000, 0.000000),
		vec3(0.000000, 0.000000, 0.000000), vec3(0.000000, 1.000000, 0.000000),

		vec3(0.000000, 0.000000, 1.000000), vec3(0.000000, 1.000000, 0.000000),
		vec3(1.000000, 0.000000, 1.000000), vec3(0.000000, 1.000000, 0.000000),
		vec3(1.000000, 0.000000, 0.000000), vec3(0.000000, 1.000000, 0.000000),

		vec3(0.000000, 1.000000, 1.000000), vec3(0.000000, -1.000000, 0.000000),
		vec3(0.000000, 1.000000, 0.000000), vec3(0.000000, -1.000000, 0.000000),
		vec3(1.000000, 1.000000, 0.000000), vec3(0.000000, -1.000000, 0.000000),

		vec3(0.000000, 1.000000, 1.000000), vec3(0.000000, -1.000000, 0.000000),
		vec3(1.000000, 1.000000, 0.000000), vec3(0.000000, -1.000000, 0.000000),
		vec3(1.000000, 1.000000, 1.000000), vec3(0.000000, -1.000000, 0.000000)
	};

	for (int i = 0; i < 72; i+=2) {
		finalBoxData[i] = finalBoxData[i] * boxSize;
	}





	GLuint boxVBO, boxVAO;


	glGenBuffers(1, &boxVBO);
	glGenVertexArrays(1, &boxVAO);
	glBindVertexArray(boxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(finalBoxData), finalBoxData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(fvec3)*2, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(fvec3)*2, (void*)sizeof(fvec3));
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	int fps = 30;

	double deltaTime = 1.0 / double(fps);
	double currentFrame = 0.0;
	glPointSize(5.0f);
	while(!glfwWindowShouldClose(window)) {

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		frameCounter++;
		currentFrame += deltaTime;
		

		int32_t lookingIndex = 13;


		float theRange = 3.0f;



		makeKDTree(initialParticles, numberOfPoints, kdConCon);

		


		float floatDeltaTime = float(deltaTime);

		status = clSetKernelArg(computeDPKernel, 2, sizeof(float), &floatDeltaTime);
		if (status)printf("dp kernel 2 %i\n", status);
		status = clSetKernelArg(computeDPKernel, 1, sizeof(cl_mem), &clTheTree);
		if (status)printf("dp kernel 1 %i\n", status);

		size_t globalWorkSize[3] = {numberOfPoints, 1, 1 };
		cl_event* kernelEvent = new cl_event();
		status = clEnqueueNDRangeKernel(cmdQueue, computeDPKernel, 1, NULL, globalWorkSize, NULL, 0, NULL, kernelEvent);
		if (status)printf("cd kernel execution %i\n", status);
		clWaitForEvents(1, kernelEvent);
		clFinish(*kdConCon.cmdQueue);


		vec3 eye = vec3(sin(currentFrame/2.0f) * boxSize.x, boxSize.y/2.0f, cos(currentFrame/2.0f) * boxSize.z) + (boxSize/2.0f);

		//eye = vec3(boxSize.x, boxSize.y / 2.0f, boxSize.z) + (boxSize / 2.0f);


		vec3 at(boxSize/2.0f);

		mat4 view = glm::lookAt(eye, at, vec3(0, 1, 0));

		mat4 proj = glm::perspective(glm::radians(70.0f), float(frameRatio), 0.1f, 200.0f);

		glEnable(GL_DEPTH_TEST);
		boxShader.use();
		boxShader.setMatFour("view", view);
		boxShader.setMatFour("projection", proj);

		glBindVertexArray(boxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36); 
		glBindVertexArray(0);


		glDisable(GL_DEPTH_TEST);
		waterShader.use();
		waterShader.setMatFour("view", view);
		waterShader.setMatFour("projection", proj);

		glBindVertexArray(particleVAO);
		glDrawArrays(GL_POINTS, 0, numberOfPoints);
		glBindVertexArray(0);


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


