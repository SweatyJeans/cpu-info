# cpu-info

**cpu-info** is a minimal **x86 CPU feature detection** library written in C and NASM.

It provides an easy way to query CPU capabilities such as SSE, AVX, AVX2 and AVX512F,
including whether the operating system actually enables these instruction sets.

The library is designed to have **almost zero runtime overhead**:
only a single initialization function (`InitCPUInfo`) is required.

**NOTE** > Currently, cpu-info is **only compatible with Linux**.


**Features**:

The following macros are currently supported:

* **Macros** for various common vendor strings (short ID identifying the CPU manufacturer)
    * CPU_INFO_PROCESSOR_AMD
    * CPU_INFO_PROCESSOR_IDT
    * CPU_INFO_PROCESSOR_CYRIX
    * CPU_INFO_PROCESSOR_INTEL
    * CPU_INFO_PROCESSOR_INTEL_RARE
    * CPU_INFO_PROCESSOR_TRANSMETA1
    * CPU_INFO_PROCESSOR_TRANSMETA2

* **Macros** for the number of: "threads per core," "logical cores", "physical cores" and whether "hyperthreading" is used (true or false)

* **Macros for checking CPU features**:
    * SSE
    * SSE2
    * SSE3
    * AES-NI
    * XSAVE
    * OSXSAVE
    * AVX (CPU side)
    * RDRAND
    * AVX-2 (CPU side)
    * AVX-512f (CPU side)
    * AVX (CPU and OS side)
    * AVX-2 (CPU and OS side)
    * AVX-512f (CPU and OS side)
    * SHA-1
    * SHA-256
    * HYPERVISOR (Check whether a hypervisor is active)

* The **structure** *cpu_info* (struct cpu_info) with the variables:
    * vendor_id (The determined ID of the CPU manufacturer)
    * brand_string (A more precise description of the installed CPU)
    * hypervisor_id (The ID of the hypervisor, if one is present -> if not 0)
    * physical_cores (The macro “CPU_INFO_PHYSICAL_CORES” also only accesses this variable)


**More detailed explanation and an example program**:

The purpose of my “cpu-info” project is very simple: to provide users/programmers with the most intuitive way possible to view the “capabilities” of their CPU and evaluate them at program runtime. Whether you are writing a program that requires the CPU and operating system to support AVX (AVX, AVX2, or AVX512F) and the program needs to know whether all requirements are met before it starts executing the code, or you simply want to know what features your CPU supports, cpu-info offers you all this and more in the future! There are other projects that aim to do similar things, but especially in the case of AVX detection, for example, these programs only check whether the CPU supports AVX, which does not necessarily mean that the operating system also allows these instructions. cpu-info also determines this. Furthermore, one of my goals was to generate as little overhead as possible, for example, by avoiding many function calls. Therefore, only a single function is required for the entire functionality of cpu-info, which only needs to be called once. But more on that in the next example program.

**Example**:

```c
#include <ci_include/cpuinfo.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    struct cpu_info info;
    InitCPUInfo(&info);

    if (strcmp(info.vendor_id, CPU_INFO_PROCESSOR_AMD) == 0) printf("AMD DETECTED\n");

    printf("CPU features AVX-2: %s\nOS features AVX-2: %s\n", CPU_INFO_AVX2_CPU_SIDE(info) ? "yes" : "no", CPU_INFO_AVX2(info) ? "yes" : "no");

    return 0;
}
```

This program only requires the header **cpuinfo.h** from the cpu-info include directory (**ci_include**) and, of course, the headers that your program requires. To begin with, create a cpu_info structure and initialize it with InitCPUInfo -> This is the one function you need. The following program uses three macros defined in cpuinfo.h. They always begin with CPU_INFO_ followed by their task, so the value after CPU_INFO_PROCESSOR_AMD is simply the string “AuthenticAMD,” which identifies AMD processors. The variable used in the if statement: vendor_id of the cpu_info structure stores the identification number determined for your CPU. The printf statement then outputs whether the CPU supports AVX-2 and whether the OS also allows AVX-2. NOTE CPU_INFO_AVX2_CPU_SIDE checks whether the CPU allows the use of AVX-2. I could have also called the other macro CPU_INFO_AVX2 CPU_INFO_AVX2_OS_SIDE, but CPU_INFO_AVX2 checks not only the OS side but both, If CPU_INFO_AVX2, CPU_INFO_AVX, or CPU_INFO_AVX512F (and the following AVX-related macros in the next updates) return 1, i.e., true, this always applies to both the CPU and the OS. All other macros are designed in the same way and are all very easy to understand.



**Install**

To install cpu-info you can run the *Makefile*. You can install it local with:
```bash
make
```
Or system wide with:
```bash
sudo make install
```
This installs:
**Header** → */usr/local/include/ci_include/cpuinfo.h*
**Library** → */usr/local/lib/libcpuinfo.a*

**Compiling**
To compile a program that uses *ci_include/cpuinfo.h* you need to link the library as follows (in my example I use clang as a compiler):
```bash
clang -no-pie your_program.c -o your_program -l:libcpuinfo.a
```

Of course, I would be very happy to receive feedback and suggestions :) I hope that this project will save some of you a bit of typing work.