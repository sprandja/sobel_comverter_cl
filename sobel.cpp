#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
// STB Image libraries (download these headers separately)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


#ifdef  __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

const char  cl_kernel_sobel_filename[]    = "sobel_image.cl";

static int 
read_kernel_from_file(const char *filename, char **source, size_t *len)
{
    struct stat statbuf;
    FILE        *fh;
    size_t      file_len;
    
    fh = fopen(filename, "r");
    if (fh == 0)
        return -1;
    
    stat(filename, &statbuf);
    file_len = (size_t)statbuf.st_size;
    *len = file_len;
    *source = (char *) malloc(file_len+1);
    fread(*source, file_len, 1, fh);
    (*source)[file_len] = '\0';
    
    fclose(fh);
    return 0;
}

int main(int argc, char** argv){
    cl_device_id        device;
    cl_context          context;
    cl_command_queue    queue;
    int                 err;
    cl_device_type      device_type = CL_DEVICE_TYPE_GPU;


    int width, height, channels;
    size_t srclen[1];
    char *source[1];
    
    // 1. Load the image using STB (forcing 4 channels for RGBA)
    unsigned char *host_image = stbi_load("input.png", &width, &height, &channels, 4);
    if (!host_image) {
        printf("Failed to load input.png\n");
        return 1;
    }

#if (__APPLE__) || defined(__MACOSX)
    cl_platform_id platform = NULL;
#else
    cl_platform_id platform = NULL;
    err = clGetPlatformIDs(1, &platform, NULL);
    if(err != CL_SUCCESS)
    {
        printf("clGetPlatformIDs() failed. (%d)\n", err);
        return EXIT_FAILURE;
    }
#endif

    err = clGetDeviceIDs(platform, device_type, 1, &device, NULL);
    if(err != CL_SUCCESS)
    {
        printf("clGetDeviceIDs() failed. (%d)\n", err);
        return EXIT_FAILURE;
    }

    char    deviceName[ 512 ];
    char    deviceVendor[ 512 ];
    char    deviceVersion[ 512 ];
    err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof( deviceVendor ),
                           deviceVendor, NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof( deviceName ),
                          deviceName, NULL);
    err |= clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof( deviceVersion ),
                           deviceVersion, NULL);

    printf("OpenCL Device Vendor = %s,  OpenCL Device Name = %s,  OpenCL Device Version = %s\n", deviceVendor, deviceName, deviceVersion);

    context = clCreateContext( 0, 1, &device, NULL, NULL, &err);
    if (!context || err)
    {
        printf("clCreateContext() failed. (%d)\n", err);
        return EXIT_FAILURE;
    }

    queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    if(!queue || err)
    {
        printf("clCreateCommandQueue() failed. (%d)\n", err);
        return EXIT_FAILURE;
    }

    err |= read_kernel_from_file(cl_kernel_sobel_filename, &source[0], &srclen[0]);
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)source, (size_t *)srclen, &err);

    if (clBuildProgram(program, 1, &device, NULL, NULL, NULL) != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Kernel Build Error:\n%s\n", log);
        free(log);
        return 1;
    }

    cl_kernel kernel = clCreateKernel(program, "sobel_rgb", NULL);



}