#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>
#include <windows.h>
#include <stdio.h>

using namespace std;
using namespace cl;

struct vecn {
	vector<double> cords;
};
struct face {
	vector<int> v;
};

Program CreateProgram(const string&);
double  Clocling(clock_t);
void    console();
void    openfilecl();

HWND console_handle;
HANDLE hConsole;
HDC device_context;
RECT rect;
CONSOLE_FONT_INFO GETFONT;
COORD font_size;
char* source_str;

static const char source[] =
"#if defined(cl_khr_fp64)\n"
"#  pragma OPENCL EXTENSION cl_khr_fp64: enable\n"
"#elif defined(cl_amd_fp64)\n"
"#  pragma OPENCL EXTENSION cl_amd_fp64: enable\n"
"#else\n"
"#  error double precision is not supported\n"
"#endif\n"
"kernel void add(\n"
"       ulong n,\n"
"       global const double *a,\n"
"       global const double *b,\n"
"       global double *c\n"
"       )\n"
"{\n"
"    size_t i = get_global_id(0);\n"
"    if (i < n) {\n"
"       c[i] = a[i] + b[i];\n"
"    }\n"
"}\n";

union _vecn
{
	cl_int aaa[2];
};
union _face
{
	_vecn bbb[100];
	//vector<_vecn> bbb;
	//_vecn* bbb;
};

int main() {
	openfilecl();
	console();

	/*cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret |= clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
	// Create an OpenCL context
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	int work = 1001;
	int sum = 100;
	while (work < 10000001) {
		face f;
		for (int i = 0; i < work; i++) {
			std::random_device rd;
			std::mt19937 mt(rd());
			std::uniform_int_distribution<int> distribution(-100000, 100000);

			f.v.push_back(i);
		}

		clock_t start = clock();

		cl::Program program = CreateProgram("array.cl");
		auto context = program.getInfo<CL_PROGRAM_CONTEXT>();
		auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
		auto& device = devices.front();

		cl::Kernel kernel(program, "array");
		cl_int err = 0;
		int WorkGroupSize = kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device, &err);

		WorkGroupSize = 2;
		//cout << WorkGroupSize << '\n';
		int NumWorkGroup = f.v.size() / WorkGroupSize;

		cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int) * f.v.size(), f.v.data());
		cl::Buffer outBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(int) * NumWorkGroup);

		kernel.setArg(0, inBuf);
		kernel.setArg(1, sizeof(int) * WorkGroupSize, nullptr);
		kernel.setArg(2, outBuf);

		vector<int> outVec(f.v.size());

		cl::CommandQueue queue(context, device);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(f.v.size()), cl::NDRange(WorkGroupSize));
		queue.enqueueReadBuffer(outBuf, GL_TRUE, 0, sizeof(int) * outVec.size(), outVec.data());

		double a = Clocling(start);
		SetPixel(device_context, 10 + sum, 100 - a, RGB(255, 0, 0));
		for (int i = 0; i < outVec.size(); i++) {
			outVec[i] *= 2;
		}
		cout << outVec[3];
		cout << ":GPU ";
		start = clock();
		long long sums = 0;
		for (int i = 0; i < work; i++) {
			sums += f.v[i];
		}
		cout << sums << ":CPU \n";

		double b = Clocling(start);
		SetPixel(device_context, 10 + sum, 100 - b, RGB(0, 255, 0));
		sum++;
		work += 2;
		cin.get();

	}
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);*/

	openfilecl();
	cl::Program program;
	cl::Context context;
	cl::CommandQueue queue;
	try {
		std::vector<cl::Platform> platform;
		cl::Platform::get(&platform);
		if (platform.empty()) {
			std::cerr << "OpenCL platforms not found." << std::endl;
			return 1;
		}

		std::vector<cl::Device> device;
		for (auto p = platform.begin(); device.empty() && p != platform.end(); p++) {
			std::vector<cl::Device> pldev;

			try {
				p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

				for (auto d = pldev.begin(); device.empty() && d != pldev.end(); d++) {
					if (!d->getInfo<CL_DEVICE_AVAILABLE>()) continue;

					std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();

					if (
						ext.find("cl_khr_fp64") == std::string::npos &&
						ext.find("cl_amd_fp64") == std::string::npos
						) continue;

					device.push_back(*d);
					context = cl::Context(device);
				}
			}
			catch (...) {
				device.clear();
			}
		}
		if (device.empty()) {
			std::cerr << "GPUs with double precision not found." << std::endl;
			return 1;
		}

		std::cout << device[0].getInfo<CL_DEVICE_NAME>() << std::endl;
		// Create command queue.
		queue = cl::CommandQueue(context, device[0]);
		// Compile OpenCL program for found device.

		ifstream helloworldfile("array.cl");																    //file stream main.cl   --- programm for GPU
		string src(istreambuf_iterator<char>(helloworldfile), (istreambuf_iterator<char>()));			//text of program
		const char* source_str = src.c_str();
		program = cl::Program(context, cl::Program::Sources(
			1, std::make_pair(source_str, strlen(source_str))
		));
		try {
			program.build(device);
		}
		catch (const cl::Error&) {
			std::cerr
				<< "OpenCL compilation error" << std::endl
				<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0])
				<< std::endl;
			return 1;
		}

	}
	catch (const cl::Error& err) {
		std::cerr
			<< "OpenCL error: "
			<< err.what() << "(" << err.err() << ")"
			<< std::endl;
		return 1;
	}

	long long work = 1000;
	int sum = 0;
	int lasty = 500;
	int lastgy = 500;
	const long long work_end = 1 << 25;

	HPEN penc = CreatePen(PS_SOLID, 1, RGB(128, 64, 255));
	HPEN peng = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	HPEN pend = CreatePen(PS_SOLID, 0, RGB(0, 0, 0));

	SelectObject(device_context, penc);
	LineTo(device_context, 500, -100);
	LineTo(device_context, 500, 600);
	LineTo(device_context, 500, 500);
	SelectObject(device_context, peng);
	LineTo(device_context, 600, 500);
	LineTo(device_context, 0, 500);

	//SelectObject(device_context, penc);
	LineTo(device_context, 0, 500);

	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
	cout << right << std::setw(50)
		<< "# CPU\n";
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
	cout<< std::setw(50)
		<< "# GPU\n";
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	cout << std::setw(55)
		<< "Time\n\n";
	cout << std::setw(30)
		<< "WorkSize\n";

	while (work < work_end) {
		//const std::size_t N = 1 << 20;
		std::vector<double> a(work);
		std::vector<double> b(work);
		std::vector<double> c(work);
		for (int i = 0; i < work; i++) {
			std::random_device rd;
			std::mt19937 mt(rd());
			std::uniform_int_distribution<int> distribution(-100000, 100000);

			a[i] = distribution(mt);
			b[i] = distribution(mt);
		}

		clock_t start;

		start = clock();
		cl::Kernel add(program, "add");
		cl::Kernel fstruct(program, "fstruct");
		// Prepare input data.
		// Allocate device buffers and transfer input data to device.
		cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			a.size() * sizeof(double), a.data());

		cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			b.size() * sizeof(double), b.data());

		cl::Buffer C(context, CL_MEM_READ_WRITE,
			c.size() * sizeof(double));

		// Set kernel parameters.
		add.setArg(0, static_cast<cl_ulong>(work));
		add.setArg(1, A);
		add.setArg(2, B);
		add.setArg(3, C);

		_vecn vec;
		vec.aaa[0] = 1;
		vec.aaa[1] = 2;
		_face* f = new _face[2];
		f[0].bbb[0] = vec;
		vec.aaa[0] = 3;
		vec.aaa[1] = 4;
		f[0].bbb[1] = vec;

		cl::Buffer D(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(f) * 2, f);
		fstruct.setArg(0, D);
		// Launch kernel on the compute device.
		queue.enqueueNDRangeKernel(fstruct, cl::NullRange, cl::NDRange(work), cl::NullRange);
		delete[] f;
		//queue.enqueueNDRangeKernel(fstruct, cl::NullRange, cl::NullRange, cl::NullRange);
		// Get result back to host.
		//queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());
		// Should get '3' here.
		//queue.enqueueNDRangeKernel(fstruct, cl::NullRange, cl::NullRange, cl::NullRange);
		
		double speed = Clocling(start) / 10;
		//cout << "Group: " << work << '\n';
		//cout << speed << ": GPU\n";
		SelectObject(device_context, peng);
		LineTo(device_context, sum, 500 - speed);
		MoveToEx(device_context, sum - 10, lasty, NULL);
		//LineTo(device_context, sum - 10, lastgy);
		lastgy = 500 - speed;

		clock_t start2 = clock();
		for (int i = 0; i < work; i++) {
			for (int j = 0; j < 10; j++) {
				c[i] = a[i] * b[i] * a[i] * b[i];
				c[i] /= a[i] * b[i] * a[i] * b[i];
			}
		}

		double speed2 = Clocling(start2) / 10;
		//cout << speed2 << ": CPU\n";
		SelectObject(device_context, penc);
		LineTo(device_context, sum, 500 - speed2);
		MoveToEx(device_context, sum, lastgy, NULL);
		//LineTo(device_context, sum - 10, lasty);
		lasty = 500 - speed2;
		work += 10000;
		sum += 10;
	}
	getchar();
	ReleaseDC(console_handle, device_context);
	return 0;
}

Program CreateProgram(const string& file) {
	vector<Platform> platforms;																		//array of platforms (implementation sdks)
	Platform::get(&platforms);																		//filling platforms
																									//
	auto platform = platforms.front();																//first platform
	vector<Device> devices;																			//create device struct
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);												//get device type GPU
																									//
	auto device = devices.front();																	//get first device
	auto vendor = device.getInfo<CL_DEVICE_VENDOR>();												//get info vendor (AMD)
	auto version = device.getInfo<CL_DEVICE_VERSION>();												//get info version
																									//
	ifstream helloworldfile(file);																    //file stream main.cl   --- programm for GPU
	string src(istreambuf_iterator<char>(helloworldfile), (istreambuf_iterator<char>()));			//text of program
																									//
	Program::Sources sources(1, make_pair(src.c_str(), src.length() + 1));							//
	Context context(device);																		//list of devices
	Program program(context, sources);																//create program
																									//
	program.build("-cl-std=CL1.2");																	//building program
	return program;
}
double  Clocling(clock_t start) {
	clock_t end = clock();
	double seconds = ((double)end - (double)start);
	//printf("%.20f\n", seconds);
	return seconds * 10;
}
void    console() {
	console_handle = GetConsoleWindow();
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	device_context = GetDC(console_handle);
	GetCurrentConsoleFont(hConsole, true, &GETFONT);
	font_size = GetConsoleFontSize(hConsole, GETFONT.nFont);
	GetWindowRect(console_handle, &rect);
	rect.top *= font_size.Y / 3;
}
void    openfilecl() {
	FILE* fp;
	std::size_t source_size;

	fopen_s(&fp, "array.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(0x100000);
	source_size = fread(source_str, 1, 0x100000, fp);
	fclose(fp);
}