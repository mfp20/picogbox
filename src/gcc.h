#ifndef GCC_HH
#define GCC_HH

#define UNUSED __attribute__((unused))
#define INLINE __attribute__((always_inline))
#define WEAK __attribute__((weak))
#define ISR __attribute__((interrupt))
#define ISRH __attribute__((interrupt_handler))

#define NOP() __asm__ __volatile__("nop")

#endif
