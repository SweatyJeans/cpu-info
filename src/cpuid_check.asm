default rel

SECTION .data
    maxLeafBelow7ErrMsg:    db "Error > Leaf 7 not supported by CPU. CPUINFO will be updated so your CPU doesen't need to support leaf 7 and above.",10,0
    maxLeafBelow7ErrMsgLen: equ $ - maxLeafBelow7ErrMsg

SECTION .bss
    xcr0_result: resq 1 ;to safe the result of ecx=0 -> xgetbv
    ;XCR0 bit 1 → XMM
    ;XCR0 bit 2 → YMM
    ;XCR0 bit 5 → opmask registers
    ;XCR0 bit 6 → ZMM_hi256
    ;XCR0 bit 7 → hi16_ZMM

    eax_max_leaf: resd 1

SECTION .text
    GLOBAL InitCPUInfo
    InitCPUInfo:
        ;rdi = ptr to struct cpu_info
        TEST rdi,rdi
        jz .err_return ;return if ptr is zero

        XOR eax,eax
        CPUID

        MOV [eax_max_leaf],eax

        CMP eax,7 ;check if leave 7 and above are available
        JNB .start_at_vendor_id_string_and_continue

        ;error msg
        MOV rax,1
        MOV rdi,2
        MOV rsi,maxLeafBelow7ErrMsg
        MOV rdi,maxLeafBelow7ErrMsgLen

        syscall 
        ret ;return to caller

        .start_at_vendor_id_string_and_continue:

        ;safe vendor id string
        MOV [rdi],ebx
        MOV [rdi+4],edx
        MOV [rdi+8],ecx
        MOV BYTE [rdi+12],0

        ;SSE,SSE2,SSE3,AVX,AES etc.
        MOV eax,1
        cpuid

        MOV [rdi+16],eax
        MOV [rdi+20],ebx
        MOV [rdi+24],ecx
        MOV [rdi+28],edx

        MOV BYTE [rdi+13],0 ;clear avx_support flags -> each flag will be set later

        bt ecx,27

        JNC .continue_without_xgetbv_test

        OR BYTE [rdi+13],2 ;bit 27 in ecx is set osxsave is available

        BT ecx,28

        JNC .continue_without_xgetbv_test

        OR BYTE [rdi+13],4 ;bit 28 in ecx is set so the CPU itself supports AVX instructions

        PUSH rcx
        PUSH rax
        PUSH rdx

        XOR ecx,ecx
        XGETBV ;eax = xcr0 low and edx = xcr0 high

        ;safe XGETBV result for later use
        mov [xcr0_result],eax
        mov [xcr0_result+4],edx

        AND eax,6 ;check bits 1 (XMM regs) and 2 (YMM regs)
        CMP eax,6 ;eax must be exactly 6 so that both bits are enabled

        JNE .os_does_not_support_avx

        OR BYTE [rdi+13],1 ;set bit 1 from avx_support flags to signal that AVX ist completely available by the CPU and the OS

        .os_does_not_support_avx:

        POP rdx
        POP rax
        POP rcx
        
        .continue_without_xgetbv_test:

        MOV eax,7
        XOR ecx,ecx
        cpuid

        MOV [rdi+36],ebx
        MOV [rdi+40],ecx
        MOV [rdi+44],edx
        
        TEST BYTE [rdi+13],2 ;Checks for AVX support CPU and OS sided -> If not set neither AVX nor AVX2 is supported
        JZ .avx2_is_not_supported

        BT ebx,5 ;Check if CPU supports AVX2

        JNC .avx2_is_not_supported

        OR BYTE [rdi+13],8 ;Set bit 4 of avx_support when AVX2 is supported

        .avx2_is_not_supported:

        TEST BYTE [rdi+13],4

        JZ .avx512f_is_not_supported ;if bit 2 from avx_support is not set osxsave is not supported and AVX512F needs OSXSAVE

        BT ebx,16 ;check if the CPU supports AVX512-f (AVX512 Foundation)

        JNC .avx512f_is_not_supported

        ;AVX512-f needs bit 1, 2, 5, 6, 7 of xcr0
        MOV rax,[xcr0_result]
        AND rax,0xE6 ;0xE6 = bitmask to test bits 1, 2, 5, 6, 7 in xcr0

        CMP rax,0xE6

        JNE .avx512f_is_not_supported 

        OR BYTE [rdi+13],16 ;Set bit 5 of avx_support when AVX512-f is supported

        .avx512f_is_not_supported:

        MOV eax,0x80000002
        CPUID 

        MOV [rdi+48],eax
        MOV [rdi+52],ebx
        MOV [rdi+56],ecx
        MOV [rdi+60],edx

        MOV eax,0x80000003
        CPUID 

        MOV [rdi+64],eax
        MOV [rdi+68],ebx
        MOV [rdi+72],ecx
        MOV [rdi+76],edx

        MOV eax,0x80000004
        CPUID 

        MOV [rdi+80],eax
        MOV [rdi+84],ebx
        MOV [rdi+88],ecx
        MOV [rdi+92],edx

        MOV BYTE [rdi+96],0 

        ;hypervisor id string
        TEST DWORD [rdi+24],(1<<31)

        MOV BYTE [rdi+99],0 ;initialize the hypervisor id string to 0

        JZ .no_hypervisor_id_string

        MOV eax,0x40000000
        CPUID 

        MOV [rdi+99],ebx 
        MOV [rdi+103],ecx 
        MOV [rdi+107],edx 
        MOV BYTE [rdi+111],0

        .no_hypervisor_id_string:

        
        MOV eax,[eax_max_leaf]
        CMP eax,0xB ;check if leaf 0xB is available -> if it's not return to caller, in later updates fix this with a fallback fo getting the cores

        JB .leaf_0xB_not_available_use_fallback

        ;pyhsical and logical cores (NOTE > muss noch am anfang machen das geguckt wird ob das programm auf nem intel läuft)
        MOV eax,0xB
        XOR ecx,ecx 
        CPUID 

        MOV r8d,ebx ;safe threads per core
        MOV [rdi+112],ebx

        MOV eax,0xB
        MOV ecx,1
        CPUID 

        MOV eax,ebx ;safe logical processors
        MOV [rdi+116],ebx 

        ;physical core = logical cores / threads per core
        XOR edx,edx
        DIV r8d
        MOV [rdi+120],eax

        .leaf_0xB_not_available_use_fallback:

        ret

        .err_return:
        ret
