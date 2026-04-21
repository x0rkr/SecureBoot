#ifndef INTEGRITY_H
#define INTEGRITY_H

/* Timing threshold for debugger detection
   Adjust if getting false positives on slow hardware */
#define TIMING_THRESHOLD 5000000

/* VM detection result codes */
#define VM_NOT_DETECTED  0
#define VM_DETECTED      1

/* Function Prototypes */
int detect_vm(void);
int detect_timing_anomaly(void);
int run_integrity_checks(void);

#endif

