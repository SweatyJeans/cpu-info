#pragma once

/*
* Hey, thank you for downloading my project. It's small but I think when you need to know if your CPU supports a feature like for example
* AVX, AVX2 oder AVX512 (or many others but more on this later) and you don't whant to implement CPUID by yourself then this project is
* the only thing you need.
*
* It's pretty simple, you need to create a cpu_info structure and call the InitCPUInfo function. After those 2 steps you can easily
* use the macros at the bottom to look for a CPU feature. A Quick example. The following program tests if you can safely use SSE2
*
* #include <ci_include/cpuinfo.h>
*
* int main(void) {
*
*     struct cpu_info info = {0};
*     InitCPUInfo(&info);
*
*     if (CPU_INFO_SSE2(info)) {
*         printf("SSE2 is available.\n");
*     }
*
*     return 0;
* }
*
* That's it.
*/

#include <stdint.h>

/*
* This structure safes all by CPUID gathered information that is currenty available through this "API".
* It's everything you need to work with the API macros.
*/
struct cpu_info {
    char vendor_id[13];

    /*
    * The parameter <avx_support> is more likely for internal use in the asm source code as well as for the AVX_SUPPORT TEST macros.
    * But ofcource you can read the bits by yourself, therefore I put the discription for each bit below.
    *
    * NOTE > To change the value of <avx_support> can cause many issues or in the best case a wrong output by a macro, remember the source code directly reads the bits and works with them.
    *
    * Bit 1 -> OS AND CPU SUPPORT AVX, 
    * Bit 2 -> OSXSAVE IS SUPPORTED, 
    * Bit 3 -> CPU SUPPORTS AVX, 
    * Bit 4 -> AVX2 IS SUPPORTED BY THE CPU AND THE OS, 
    * Bit 5 -> AVX512-f IS SUPPORTED BY THE CPU AND THE OS
    */
    uint8_t avx_support; 

    char p1[2]; //just padding

    /*
    * The naming convention of the following leaf variables is as follow.
    * It starts with "leaf" followed by the leaf number used in the CPUID instruction
    * followed by the register.
    *
    * So for example:
    *
    *   MOV eax,1 -> Leaf value 1
    *   CPUID
    *   MOV [leaf1_eax],eax
    *
    * I hope it's clear what I mean but I guess it's pretty self explaining :)
    */
    uint32_t leaf1_eax;
    uint32_t leaf1_ebx;
    uint32_t leaf1_ecx;
    uint32_t leaf1_edx;
    //uint8_t avx_support;

    uint32_t leaf7_eax;
    uint32_t leaf7_ebx;
    uint32_t leaf7_ecx;
    uint32_t leaf7_edx;

    char brand_string[49]; //The buffer for the brand string. NOTE > The string ist terminated with 0
    char p2[2]; //just padding again here

    /*
    * When BIt 31 in leaf1_ecx is set (use CPU_INFO_HYPERVISOR() to check) a hypervisor is active.
    * In this case you can read <hypervisor_id> to see the "name" of the hypervisor.
    * If no hypervisor is active, <hypervisor_id> is set to 0.
    *
    * NOTE > Not all hypervisors provide a hypervisor ID string.
    */
    char hypervisor_id[13];

    uint32_t leaf0xB_subLeaf0_ebx; //used for threads per core (in this version Intel processors only)
    uint32_t leaf0xB_subLeaf1_ebx; //used for logical cores (in this version Intel processors only)
    uint32_t physical_cores; //(in this version Intel processors only)
};

extern void InitCPUInfo(struct cpu_info* info);

//Some vendor_id's
#define CPU_INFO_PROCESSOR_AMD                      "AuthenticAMD"
#define CPU_INFO_PROCESSOR_IDT                      "CentaurHauls"
#define CPU_INFO_PROCESSOR_CYRIX                    "CyrixInstead"
#define CPU_INFO_PROCESSOR_INTEL                    "GenuineIntel"
#define CPU_INFO_PROCESSOR_INTEL_RARE               "GenuineIotel"
#define CPU_INFO_PROCESSOR_TRANSMETA1               "TransmetaCPU"
#define CPU_INFO_PROCESSOR_TRANSMETA2               "GenuineTMx86"


#define CPU_INFO_THREADS_PER_CORE(info)             (info).leaf0xB_subLeaf0_ebx
#define CPU_INFO_LOGICAL_CORES(info)                (info).leaf0xB_subLeaf1_ebx
#define CPU_INFO_PHYSICAL_CORES(info)               (info).physical_cores
#define CPU_INFO_HYPERTHREADING(info)               (CPU_INFO_THREADS_PER_CORE(info) > 1)

#define CPU_INFO_SSE(info)                          ((info).leaf1_edx & (1 << 25))
#define CPU_INFO_SSE2(info)                         ((info).leaf1_edx & (1 << 26))
#define CPU_INFO_SSE3(info)                         ((info).leaf1_ecx & 1)
#define CPU_INFO_SSSE3(info)                        ((info).leaf1_ecx & (1 << 9))
#define CPU_INFO_SSE4_1(info)                       ((info).leaf1_ecx & (1 << 19)) //SSE4.1
#define CPU_INFO_SSE4_2(info)                       ((info).leaf1_ecx & (1 << 20)) //SSE4.2
#define CPU_INFO_AESNI(info)                        ((info).leaf1_ecx & (1 << 25))
#define CPU_INFO_XSAVE(info)                        ((info).leaf1_ecx & (1 << 26))
#define CPU_INFO_OSXSAVE(info)                      ((info).leaf1_ecx & (1 << 27))
#define CPU_INFO_AVX_CPU_SIDE(info)                 ((info).leaf1_ecx & (1 << 28)) //Checks if the CPU supports AVX instructions -> that doesn't mean that ur OS provides them too
#define CPU_INFO_MMX(info)                          ((info).leaf1_edx & (1 << 23))

#define CPU_INFO_RDRAND(info)                       ((info).leaf1_ecx & (1 << 30))
#define CPU_INFO_RDSEED(info)                       ((info).leaf7_ebx & (1 << 18))
#define CPU_INFO_AVX2_CPU_SIDE(info)                ((info).leaf7_ebx & (1 << 5)) //Checks if the CPU supports AVX2 instructions (again) -> that doesn't mean that ur OS provides them too
#define CPU_INFO_AVX512F_CPU_SIDE(info)             ((info).leaf7_ebx & (1 << 16)) //Check if the CPU supports AVX512-f (again) -> that doesn't mean that ur OS provides them too
#define CPU_INFO_SHA(info)                          ((info).leaf7_ebx & (1 << 29)) //Checks for support for the SHA-1 and SHA-256 extension
#define CPU_INFO_BMI1(info)                         ((info).leaf7_ebx & (1 << 3))
#define CPU_INFO_BMI2(info)                         ((info).leaf7_ebx & (1 << 8))

#define CPU_INFO_HYPERVISOR(info)                   ((info).leaf1_ecx & (1 << 31)) //Returns 1 if a hypervisor is active

//AVX_SUPPORT TESTS
#define CPU_INFO_AVX(info)                          ((info).avx_support & 1) //NOTE > the first bit of avx_support is just set when both the CPU and the OS support AVX instructions, so when this bit is set you don't have to check the CPU support for AVX as well
#define CPU_INFO_AVX2(info)                         ((info).avx_support & (1 << 4)) //NOTE > If this macro returns 1 (true) AVX2 is completety safe to use
#define CPU_INFO_AVX512F(info)                      ((info).avx_support & (1 << 5)) //NOTE > If this macro returns 1 (true) AVX512-f (AVX512 Foundation) is completety safe to use


/*
* This function is an easy way to print out all gathered information
* In the future it's planed that you can use this project as an fast CLI tool too to this is the beginning of the CLI part
*
* You call this function with a pointer to printf and a pointer to a cpu_info structure so you need the stdio.h header
* The call looks like this:
*
* PrintCPUInfo(&printf, &info);
*/
static inline void PrintCPUInfo(int (*print_func)(const char*, ...), struct cpu_info* info) {
    print_func(
        "INFO:\n\n"
        "\t* VendorID: %s\n"
        "\t* BrandString: %s\n\t"
        "\nCORES:\n\n"
        "\t* Theads per core: %i\n"
        "\t* Physical cores: %i\n"
        "\t* Logical cores: %i\n"
        "\t* Hyperthreading: %s\n"
        "\nSSE:\n\n"
        "\t* SSE: %s\n"
        "\t* SSE2: %s\n"
        "\t* SSE3: %s\n"
        "\t* SSSE3: %s\n"
        "\t* SSE4.1: %s\n"
        "\t* SSE4.2: %s\n"
        "\nAVX:\n\n"
        "\t* AVX (OS): %s\n"
        "\t* AVX (CPU): %s\n"
        "\t* AVX-2 (OS): %s\n"
        "\t* AVX-2 (CPU): %s\n"
        "\t* AVX-512f (OS): %s\n"
        "\t* AVX-512f (CPU): %s\n"
        "\nRANDOM:\n\n"
        "\t* RDRAND: %s\n"
        "\t* RDSEED: %s\n"
        "\nCRYPTOGRAPHY:\n\n"
        "\t* AES-NI: %s\n"
        "\t* SHA (1 and 256): %s\n"
        "\nHYPERVISOR:\n\n"
        "\t* Hypervisor: %s\n"
        "\t* Hypervisor ID: %s\n"
        "\nOTHERS:\n\n"
        "\t* XSAVE: %s\n"
        "\t* OSXSAVE: %s\n"
        "\t* MMX: %s\n"
        "\t* BMI1: %s\n"
        "\t* BMI2: %s\n"
        , 
        
        info->vendor_id, 
        info->brand_string,
        CPU_INFO_THREADS_PER_CORE(*info),
        info->physical_cores,
        CPU_INFO_LOGICAL_CORES(*info),
        CPU_INFO_HYPERTHREADING(*info) ? "active" : "disabled",
        CPU_INFO_SSE(*info) ? "yes" : "no",
        CPU_INFO_SSE2(*info) ? "yes" : "no",
        CPU_INFO_SSE3(*info) ? "yes" : "no",
        CPU_INFO_SSSE3(*info) ? "yes" : "no",
        CPU_INFO_SSE4_1(*info) ? "yes" : "no",
        CPU_INFO_SSE4_2(*info) ? "yes" : "no",
        CPU_INFO_AVX(*info) ? "yes" : "no",
        CPU_INFO_AVX_CPU_SIDE(*info) ? "yes" : "no",
        CPU_INFO_AVX2(*info) ? "yes" : "no",
        CPU_INFO_AVX2_CPU_SIDE(*info) ? "yes" : "no",
        CPU_INFO_AVX512F(*info) ? "yes" : "no",
        CPU_INFO_AVX512F_CPU_SIDE(*info) ? "yes" : "no",
        CPU_INFO_RDRAND(*info) ? "yes" : "no",
        CPU_INFO_RDSEED(*info) ? "yes" : "no",
        CPU_INFO_AESNI(*info) ? "yes" : "no",
        CPU_INFO_SHA(*info) ? "yes" : "no",
        CPU_INFO_HYPERVISOR(*info) ? "active" : "disabled",
        *info->hypervisor_id ? info->hypervisor_id : "disabled",
        CPU_INFO_XSAVE(*info) ? "yes" : "no",
        CPU_INFO_OSXSAVE(*info) ? "yes" : "no",
        CPU_INFO_MMX(*info) ? "yes" : "no",
        CPU_INFO_BMI1(*info) ? "yes" : "no",
        CPU_INFO_BMI2(*info) ? "yes" : "no"

    );
}