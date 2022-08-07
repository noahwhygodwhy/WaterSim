#include "WaterSim.hpp"
#include "clTypeDefs.hpp"
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
constexpr uint32_t MAX_BALLS = 100000;
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
	cl_command_queue cmdQueue = clCreateCommandQueueWithProperties(clContext, device, qProperties, &status);
	printf("cmdqueue status: %i\n", status);

	cl_mem clVelocities = clCreateBuffer(clContext, CL_MEM_READ_WRITE, sizeof(fvec3) * MAX_BALLS, NULL, &status);

//shared context buffer


	fvec4* initialBathPositions = new fvec4[MAX_BALLS](fvec4(0));
	fvec3* initialBathVelocities = new fvec3[MAX_BALLS](fvec3(0));



	uint32_t numberOfBalls = 0;





	ivec3 counts = boxSize / (ballRad * 2);

	printf("%i, %i, %i\n", counts.x, counts.y, counts.z);



	for (int i = 0; i < MAX_BALLS; i++) {
		float xR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.x;
		float yR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.y;
		float zR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.z;
		initialBathPositions[numberOfBalls] = (fvec4(xR, yR, zR, 0));


		//
		//initialBathPositions[numberOfBalls] = fvec4((fvec3(x, y, z) * fvec3(ballRad * 2.0f)), 0.0f);

		initialBathVelocities[numberOfBalls] = fvec3(0.0f);
		numberOfBalls++;
		
	}


	//for (int x = 0; x < counts.x/2; x++) {
	//	for (int y = 0; y < (counts.y*3)/4; y++) {
	//		for (int z = 0; z < counts.z;  z++) {
	//			if (numberOfBalls < MAX_BALLS) {
	//				float xR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.x;
	//				float yR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.y;
	//				float zR = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * boxSize.z;
	//				initialBathPositions[numberOfBalls] = (fvec4(xR, yR, zR, 0));


	//				//
	//				//initialBathPositions[numberOfBalls] = fvec4((fvec3(x, y, z) * fvec3(ballRad * 2.0f)), 0.0f);

	//				initialBathVelocities[numberOfBalls] = fvec3(0.0f);
	//				numberOfBalls++;
	//			}
	//		}
	//	}
	//}

	




	GLuint ballVBO, ballVAO;
	glGenVertexArrays(1, &ballVAO);
	glBindVertexArray(ballVAO);
	glGenBuffers(1, &ballVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fvec4) * MAX_BALLS, initialBathPositions, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
	glEnableVertexAttribArray(0);

	cl_mem clPositions = clCreateFromGLBuffer(clContext, CL_MEM_READ_WRITE, ballVBO, &status);
	printf("positions clmem from VBO: %i\n", status);
	status = clEnqueueAcquireGLObjects(cmdQueue, 1, &clPositions, 0, NULL, NULL);
	printf("aquire gl object status: %i\n", status);
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




	status = clSetKernelArg(waterSimKernel, 0, sizeof(cl_mem), &clPositions);
	printf("set arg 0 status: %i\n", status);
	status = clSetKernelArg(waterSimKernel, 0, sizeof(cl_mem), &clVelocities);
	printf("set arg 1 status: %i\n", status);


	

	size_t globalWorkSize[3] = { MAX_BALLS, 1, 1 };
	size_t localWorkSize[3] = { NULL, NULL, NULL };

	Shader boxShader("boxVert.glsl", "boxFrag.glsl");
	Shader waterShader("waterVert.glsl", "waterFrag.glsl");


	uint32_t frameCounter = 0;
	float frameTimes[30](0);
	int lastSecondFrameCount = -1;



	status = clEnqueueWriteBuffer(cmdQueue, clPositions, CL_TRUE, 0, sizeof(fvec3) * MAX_BALLS, initialBathPositions, 0, NULL, NULL);
	printf("write 0 status: %i\n", status);
	status = clEnqueueWriteBuffer(cmdQueue, clVelocities, CL_TRUE, 0, sizeof(fvec3) * MAX_BALLS, initialBathVelocities, 0, NULL, NULL);
	printf("write 0 status: %i\n", status);

	
	




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


	//printf("about to pause\n");
	//cin.get();
	//printf("paused\n");

	glGenBuffers(1, &boxVBO);
	glGenVertexArrays(1, &boxVAO);
	glBindVertexArray(boxVAO);
	//std::cout << glGenBuffers << std::endl;
	printf("box vbo: %i\n", boxVBO);

	glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(finalBoxData), finalBoxData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(fvec3)*2, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(fvec3)*2, (void*)sizeof(fvec3));
	glEnableVertexAttribArray(1);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);





	//printf("about to pause\n");
	//cin.get();
	//printf("paused\n");


	fvec4* readInPositions = new fvec4[numberOfBalls]();


	glPointSize(5.0f);
	while (!glfwWindowShouldClose(window)) {







		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		frameCounter++;
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (int(currentFrame) > lastSecondFrameCount) {
			lastSecondFrameCount = int(currentFrame);
			float sum = 0;
			for (float f : frameTimes) {
				sum += f;
			}
			printf("fps: ~%f\n", sum / 30.0f);
		}
		frameTimes[frameCounter % 30] = 1.0f / float(deltaTime);







		glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fvec4) * numberOfBalls, readInPositions);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//TODO: don't initialize the array each time






		//printf("43 coords: %s\n", glm::to_string(initialBathPositions[43]).c_str());

		//int32_t lookingIndex = 0;
		int32_t lookingIndex = 9; //is missing 32 with 60 points

		//for (int lookingIndex = 0; lookingIndex < numberOfBalls; lookingIndex++) {

		initialBathPositions[lookingIndex].z = boxSize.z-( currentFrame / 2.0f);
		initialBathPositions[lookingIndex].y = currentFrame / 2.0f;
		initialBathPositions[lookingIndex].x = currentFrame / 2.0f;


		float theRange = 1.0f;
		//printf("\n\n");
		KDNode* theTree = makeKDTree(initialBathPositions, numberOfBalls);
		//printf("\n\n");
		//printTree(theTree);
		//printf("\n\n");
		//cin.get();
		//vector<int32_t> listOfClosePoints = getDotsInRange(initialBathPositions, theTree, lookingIndex, theRange);
		vector<int32_t> listOfClosePoints = vector<int32_t>();
		vector<int32_t> manualClosePoints = vector<int32_t>();

		/*fvec3 twentyFourPos = initialBathPositions[lookingIndex].xyz;
		for (int32_t i = 0; i < numberOfBalls; i++) {
			if (i != lookingIndex) {
				fvec3 otherPos = initialBathPositions[i];
				float d = glm::distance(twentyFourPos, otherPos);
				if (d <= theRange) {
					manualClosePoints.push_back(i);
				}
			}
		}

		std::sort(listOfClosePoints.begin(), listOfClosePoints.end());
		std::sort(manualClosePoints.begin(), manualClosePoints.end());

		if (listOfClosePoints.size() != manualClosePoints.size()) {
			for (uint asdf = 0; asdf < listOfClosePoints.size(); asdf++) {
				if (listOfClosePoints[asdf] != manualClosePoints[asdf]) {
					printf("looking index: %i\n", lookingIndex);
					printf("kdneighbors:\n");
					for (auto k : listOfClosePoints) {
						printf("%i, ", k);
					}
					printf("\n");
					printf("bruteforce:\n");
					for (auto k : manualClosePoints) {
						printf("%i, ", k);
					}
					printf("\n");
					cin.get();
				}
			}
		}*/

		//}






		/*printf("kdneighbors:\n");
		for (auto k : listOfClosePoints) {
			printf("%i, ", k);
		}
		printf("\n");
		printf("bruteforce:\n");
		for (auto k : manualClosePoints) {
			printf("%i, ", k);
		}
		printf("\n");*/



		for (int32_t i = 0; i < numberOfBalls; i++) {
			initialBathPositions[i].w = 0.0f;
		}

		for (int32_t k : listOfClosePoints) {
			initialBathPositions[k].w = 1.0f;
		}
		initialBathPositions[lookingIndex].w = 0.69f;

		initialBathPositions[32].w = 0.4f;


		glBindVertexArray(ballVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(fvec4)* MAX_BALLS, initialBathPositions, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//printTree(theTree);



		////set the frame counter argument
		//status = clSetKernelArg(waterSimKernel, 7, sizeof(cl_uint), &frameCounter);
		//printf("set arg 7 status: %i\n", status);
		//
		//status = clEnqueueWriteBuffer(cmdQueue, clToneMapData, CL_TRUE, 0, sizeof(ToneMapStruct), &toneMapData, 0, NULL,NULL);
		//printf("write tonemap status: %i\n", status);





		//cl_int fc = frameCounter;
		//status = clSetKernelArg(waterSimKernel, 2, sizeof(cl_int), &fc);



		//cl_event* firstPassEvent = new cl_event();
		//status = clEnqueueNDRangeKernel(cmdQueue, waterSimKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, firstPassEvent);
		//if(status != 0)printf("range kernel: %i\n", status);
		//clWaitForEvents(1, firstPassEvent);
		//cl_event* secondPassEvent = new cl_event();
		//status = clEnqueueNDRangeKernel(cmdQueue, renderKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, secondPassEvent);
		//if (status != 0)printf("range kernel: %i\n", status);
		//clWaitForEvents(1, secondPassEvent);w

		//clFinish(cmdQueue);

		//draw it to the glfw window


		vec3 eye = vec3(sin(currentFrame/2.0f) * boxSize.x, boxSize.y/2.0f, cos(currentFrame/2.0f) * boxSize.z) + (boxSize/2.0f);

		eye = vec3(boxSize.x, boxSize.y / 2.0f, boxSize.z) + (boxSize / 2.0f);


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

		glBindVertexArray(ballVAO);
		glDrawArrays(GL_POINTS, 0, MAX_BALLS);
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


