.section ".text.boot"

.global _start

_start:
    mrs     x1, mpidr_el1 //Only core 0 should be allowed to work, none else
    and     x1, x1, #3
    cbz     x1, 2f

1:  wfe
    b       1b

2:  ldr     x1, =_start //Stack pointer at top of code
    mov     sp, x1

    ldr     x1, =__bss_start //clear bss
    ldr     w2, =__bss_size

3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

4:  bl      kernel_main
    b       1b

