.global _start 

_start:
    ldr pc, =Reset_Handler
    ldr pc, =Undefined_Handler
    ldr pc, =SVC_Handler
    ldr pc, =PrefetchAbort_Handler
    ldr pc, =DataAbort_Handler
    ldr pc, =NotUsed_Handler
    ldr pc, =IRQ_Handler
    ldr pc, =FIQ_Handler

Reset_Handler: 
    /* 临时关闭 IRQ 中断 */
    cpsid i
    /* 关闭 Instruction/Data Cache 和 MMU */
    mrc p15, 0, r0, c1, c0, 0 // Read System Control Register
    bic r0, r0, #0x1 // 关闭MMU
    bic r0, r0, #0x2 // 关闭对齐错误检查
    bic r0, r0, #0x4 // 关闭 Data and unified caches
    bic r0, r0, #(0x1 << 11) // 关闭分支预测
    bic r0, r0, #(0x1 << 12) // 关闭 Instruction cache
    mcr p15, 0, r0, c1, c0, 0 // Write System Control Register

    /* 向量表基地址重映射 */
    mrc p15, 0, r0, c1, c0, 0 // Read System Control Register
    bic r0, r0, #(0x1 << 13) // 使用常规向量基地址0x0，且允许软件重映射
    mcr p15, 0, r0, c1, c0, 0 // Write System Control Register
    
    dsb // Data Synchronization Barrier
    isb // Instruction Synchronization Barrier
    ldr r0, =0x87800000 
    mcr p15, 0, r0, c12, c0, 0 // VBAR 向量基地址设置
    dsb
    isb

    /* bss段清零 */
.global __bss_start
.global __bss_end
_bss_start:
    .word __bss_start
_bss_end:
    .word __bss_end

    ldr r0, _bss_start
    ldr r1, _bss_end
    mov r2, #0
bss_loop:
    stmia r0!, {r2}
    cmp r0, r1
    ble bss_loop

    /* 设置 IRQ SYS SVC 模式下的 SP */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x12 // 进入IRQ模式
    msr cpsr, r0
    ldr sp, =0x80600000

    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x1f // 进入SYS模式
    msr cpsr, r0
    ldr sp, =0x80400000

    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x13 // 进入SVC模式
    msr cpsr, r0
    ldr sp, =0x80200000

    /* 重新使能 IRQ 中断 */
    cpsie i
    /* 跳转main函数 */
    b main

Undefined_Handler: 
    ldr r0, =Undefined_Handler
    bx r0

SVC_Handler: 
    ldr r0, =SVC_Handler
    bx r0

PrefetchAbort_Handler: 
    ldr r0, =PrefetchAbort_Handler
    bx r0

DataAbort_Handler: 
    ldr r0, =DataAbort_Handler
    bx r0

NotUsed_Handler: 
    ldr r0, =NotUsed_Handler
    bx r0

IRQ_Handler: 
    push {lr}					/* 保存lr地址 */
	push {r0-r3, r12}			/* 保存r0-r3，r12寄存器 */

	mrs r0, spsr				/* 读取spsr寄存器 */
	push {r0}					/* 保存spsr寄存器 */

	mrc p15, 4, r1, c15, c0, 0 /* 从CP15的C5寄存器内的值到R1寄存器中
								* 参考文档ARM Cortex-A(armV7)编程手册V4.0.pdf P49
								* Cortex-A7 Technical ReferenceManua.pdf P68 P138
								*/							
	add r1, r1, #0X2000			/* GIC基地址加0X2000，也就是GIC的CPU接口端基地址 */
	ldr r0, [r1, #0XC]			/* GIC的CPU接口端基地址加0X0C就是GICC_IAR寄存器，
								 * GICC_IAR寄存器保存这当前发生中断的中断号，我们要根据
								 * 这个中断号来绝对调用哪个中断服务函数
								 */
	push {r0, r1}				/* 保存r0,r1 */
	
	cps #0x13					/* 进入SVC模式，允许其他中断再次进去 */
	
	push {lr}					/* 保存SVC模式的lr寄存器 */
	ldr r2, =System_IRQHandler	/* 加载C语言中断处理函数到r2寄存器中*/
	blx r2						/* 运行C语言中断处理函数，带有一个参数，保存在R0寄存器中 */

	pop {lr}					/* 执行完C语言中断服务函数，lr出栈 */
	cps #0x12					/* 进入IRQ模式 */
	pop {r0, r1}				
	str r0, [r1, #0X10]			/* 中断执行完成，写EOIR */

	pop {r0}						
	msr spsr_cxsf, r0			/* 恢复spsr */

	pop {r0-r3, r12}			/* r0-r3,r12出栈 */
	pop {lr}					/* lr出栈 */
	subs pc, lr, #4				/* 将lr-4赋给pc */

FIQ_Handler: 
    ldr r0, =FIQ_Handler
    bx r0