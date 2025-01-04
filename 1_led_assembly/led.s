.global _start

_start:
    @ 使能GPIO时钟
    ldr r0, =0x020C4068
    ldr r1, =0xFFFFFFFF
    str r1, [r0]

    ldr r0, =0x020C406C
    str r1, [r0]

    ldr r0, =0x020C4070
    str r1, [r0]

    ldr r0, =0x020C4074
    str r1, [r0]

    ldr r0, =0x020C4078
    str r1, [r0]

    ldr r0, =0x020C407C
    str r1, [r0]

    ldr r0, =0x020C4080
    str r1, [r0]

    @ 配置IO复用为GPIO
    ldr r0, =0x020E0068
    ldr r1, =0x5
    str r1, [r0]

    @ 配置GPIO方向
    ldr r0, =0x0209C004
    ldr r1, =0x8
    str r1, [r0]

    @ 配置GPIO电器特性 使用默认值

    @ 配置GPIO输出电平 低电平点亮led
    ldr r0, =0x0209C000
    mov r1, #0
    str r1, [r0]

loop:
    b loop
