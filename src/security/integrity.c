#include "../include/integrity.h"
#include "../include/screen.h"

/* Check for hypervisor using CPUID instruction
   Returns 1 if running in a VM, 0 if real hardware */
int detect_vm(void) {
    unsigned int ecx = 0;

    /* CPUID leaf 1: bit 31 of ECX is the hypervisor present bit */
    __asm__ volatile (
        "mov $1, %%eax\n"
        "cpuid\n"
        : "=c"(ecx)
        :: "eax", "ebx", "edx"
    );

    if (ecx & (1 << 31)) {
        screen_println("[!] WARNING: Hypervisor detected!", COLOR_YELLOW);
        return 1;
    }

    /* CPUID leaf 0x40000000: get hypervisor vendor string */
    unsigned int ebx, ecx2, edx;
    __asm__ volatile (
        "mov $0x40000000, %%eax\n"
        "cpuid\n"
        : "=b"(ebx), "=c"(ecx2), "=d"(edx)
        :: "eax"
    );

    /* Check for known hypervisor signatures */
    /* KVM: "KVMKVMKVM\0\0\0" */
    if (ebx == 0x4D564B4B && ecx2 == 0x564B4D56 && edx == 0x0000004D) {
        screen_println("[!] WARNING: KVM detected!", COLOR_YELLOW);
        return 1;
    }

    /* VMware: "VMwareVMware" */
    if (ebx == 0x61774D56 && ecx2 == 0x4D566572 && edx == 0x65726177) {
        screen_println("[!] WARNING: VMware detected!", COLOR_YELLOW);
        return 1;
    }

    /* Microsoft Hyper-V: "Microsoft Hv" */
    if (ebx == 0x7263694D && ecx2 == 0x666F736F && edx == 0x76482074) {
        screen_println("[!] WARNING: Hyper-V detected!", COLOR_YELLOW);
        return 1;
    }

    return 0;
}

/* Basic timing check using RDTSC
   Returns 1 if timing anomaly detected (possible debugger) */
int detect_timing_anomaly(void) {
    unsigned int start_lo, start_hi, end_lo, end_hi;

    /* Read timestamp counter before */
    __asm__ volatile (
        "rdtsc"
        : "=a"(start_lo), "=d"(start_hi)
    );

    /* Do a small fixed workload */
    volatile int dummy = 0;
    for (int i = 0; i < 1000; i++) dummy += i;

    /* Read timestamp counter after */
    __asm__ volatile (
        "rdtsc"
        : "=a"(end_lo), "=d"(end_hi)
    );

    unsigned int elapsed = end_lo - start_lo;

    /* If elapsed cycles are suspiciously high, flag it */
    if (elapsed > 5000000) {
        screen_println("[!] WARNING: Timing anomaly detected!", COLOR_YELLOW);
        return 1;
    }

    return 0;
}

/* Run all integrity checks
   Returns 1 if all checks pass, 0 if any fail */
int run_integrity_checks(void) {
    screen_println("[*] Running integrity checks...", COLOR_BRIGHT_CYAN);

    int vm       = detect_vm();
    int timing   = detect_timing_anomaly();

    if (vm || timing) {
        screen_println("[!] Integrity checks FAILED.", COLOR_BRIGHT_RED);
        screen_println("[!] Proceeding in degraded mode.", COLOR_YELLOW);
        return 0;
    }

    screen_println("[+] All integrity checks PASSED.", COLOR_BRIGHT_GREEN);
    return 1;
}

