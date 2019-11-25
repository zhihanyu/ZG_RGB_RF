	.syntax unified
	.cpu cortex-m4
	.eabi_attribute 27, 3
	.fpu fpv4-sp-d16
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.thumb
	.file	"port.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.text.prvTaskExitError,"ax",%progbits
	.align	2
	.thumb
	.thumb_func
	.type	prvTaskExitError, %function
prvTaskExitError:
.LFB6:
	.file 1 "port.c"
	.loc 1 218 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	sub	sp, sp, #8
	.cfi_def_cfa_offset 8
	.loc 1 219 0
	movs	r3, #0
	str	r3, [sp, #4]
.LBB16:
.LBB17:
	.file 2 "./portmacro.h"
	.loc 2 195 0
@ 195 "./portmacro.h" 1
		mov r3, #80												
	msr basepri, r3											
	isb														
	dsb														

@ 0 "" 2
.LVL0:
	.thumb
.L2:
.LBE17:
.LBE16:
	.loc 1 229 0 discriminator 1
	ldr	r3, [sp, #4]
	cmp	r3, #0
	beq	.L2
	.loc 1 239 0
	add	sp, sp, #8
	.cfi_def_cfa_offset 0
	@ sp needed
	bx	lr
	.cfi_endproc
.LFE6:
	.size	prvTaskExitError, .-prvTaskExitError
	.section	.text.prvPortStartFirstTask,"ax",%progbits
	.align	2
	.thumb
	.thumb_func
	.type	prvPortStartFirstTask, %function
prvPortStartFirstTask:
.LFB8:
	.loc 1 262 0
	.cfi_startproc
	@ Naked Function: prologue and epilogue provided by programmer.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	.loc 1 267 0
@ 267 "port.c" 1
	 ldr r0, =0xE000ED08 	
 ldr r0, [r0] 			
 ldr r0, [r0] 			
 msr msp, r0			
 mov r0, #0			
 msr control, r0		
 cpsie i				
 cpsie f				
 dsb					
 isb					
 svc 0					
 nop					

@ 0 "" 2
	.loc 1 281 0
	.thumb
	.cfi_endproc
.LFE8:
	.size	prvPortStartFirstTask, .-prvPortStartFirstTask
	.section	.text.vPortEnableVFP,"ax",%progbits
	.align	2
	.thumb
	.thumb_func
	.type	vPortEnableVFP, %function
vPortEnableVFP:
.LFB17:
	.loc 1 708 0
	.cfi_startproc
	@ Naked Function: prologue and epilogue provided by programmer.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	.loc 1 709 0
@ 709 "port.c" 1
		ldr.w r0, =0xE000ED88		
	ldr r1, [r0]				
								
	orr r1, r1, #( 0xf << 20 )	
	str r1, [r0]				
	bx r14						
@ 0 "" 2
	.loc 1 718 0
	.thumb
	.cfi_endproc
.LFE17:
	.size	vPortEnableVFP, .-vPortEnableVFP
	.section	.text.pxPortInitialiseStack,"ax",%progbits
	.align	2
	.global	pxPortInitialiseStack
	.thumb
	.thumb_func
	.type	pxPortInitialiseStack, %function
pxPortInitialiseStack:
.LFB5:
	.loc 1 188 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LVL1:
	push	{r4, r5}
	.cfi_def_cfa_offset 8
	.cfi_offset 4, -8
	.cfi_offset 5, -4
	.loc 1 198 0
	bic	r1, r1, #1
.LVL2:
	.loc 1 196 0
	mov	r5, #16777216
	.loc 1 200 0
	ldr	r4, .L11
	str	r4, [r0, #-12]
	.loc 1 209 0
	mvn	r3, #2
	.loc 1 204 0
	str	r2, [r0, #-32]
	.loc 1 196 0
	stmdb	r0, {r1, r5}
.LVL3:
	.loc 1 209 0
	str	r3, [r0, #-36]
.LVL4:
	.loc 1 214 0
	pop	{r4, r5}
	.cfi_restore 5
	.cfi_restore 4
	.cfi_def_cfa_offset 0
	subs	r0, r0, #68
.LVL5:
	bx	lr
.L12:
	.align	2
.L11:
	.word	prvTaskExitError
	.cfi_endproc
.LFE5:
	.size	pxPortInitialiseStack, .-pxPortInitialiseStack
	.section	.text.vPortSVCHandler,"ax",%progbits
	.align	2
	.global	vPortSVCHandler
	.thumb
	.thumb_func
	.type	vPortSVCHandler, %function
vPortSVCHandler:
.LFB7:
	.loc 1 243 0
	.cfi_startproc
	@ Naked Function: prologue and epilogue provided by programmer.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	.loc 1 244 0
@ 244 "port.c" 1
		ldr	r3, pxCurrentTCBConst2		
	ldr r1, [r3]					
	ldr r0, [r1]					
	ldmia r0!, {r4-r11, r14}		
	msr psp, r0						
	isb								
	mov r0, #0 						
	msr	basepri, r0					
	bx r14							
									
	.align 4						
pxCurrentTCBConst2: .word pxCurrentTCB				

@ 0 "" 2
	.loc 1 258 0
	.thumb
	.cfi_endproc
.LFE7:
	.size	vPortSVCHandler, .-vPortSVCHandler
	.section	.text.vPortEndScheduler,"ax",%progbits
	.align	2
	.global	vPortEndScheduler
	.thumb
	.thumb_func
	.type	vPortEndScheduler, %function
vPortEndScheduler:
.LFB10:
	.loc 1 402 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	bx	lr
	.cfi_endproc
.LFE10:
	.size	vPortEndScheduler, .-vPortEndScheduler
	.section	.text.vPortEnterCritical,"ax",%progbits
	.align	2
	.global	vPortEnterCritical
	.thumb
	.thumb_func
	.type	vPortEnterCritical, %function
vPortEnterCritical:
.LFB11:
	.loc 1 410 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
.LBB18:
.LBB19:
	.loc 2 195 0
@ 195 "./portmacro.h" 1
		mov r3, #80												
	msr basepri, r3											
	isb														
	dsb														

@ 0 "" 2
.LVL6:
	.thumb
.LBE19:
.LBE18:
	.loc 1 412 0
	ldr	r2, .L16
	ldr	r3, [r2]
	adds	r3, r3, #1
	str	r3, [r2]
	bx	lr
.L17:
	.align	2
.L16:
	.word	.LANCHOR0
	.cfi_endproc
.LFE11:
	.size	vPortEnterCritical, .-vPortEnterCritical
	.section	.text.vPortExitCritical,"ax",%progbits
	.align	2
	.global	vPortExitCritical
	.thumb
	.thumb_func
	.type	vPortExitCritical, %function
vPortExitCritical:
.LFB12:
	.loc 1 427 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	.loc 1 429 0
	ldr	r2, .L20
	ldr	r3, [r2]
	subs	r3, r3, #1
	str	r3, [r2]
	.loc 1 430 0
	cbnz	r3, .L18
.LVL7:
.LBB20:
.LBB21:
	.loc 2 229 0
@ 229 "./portmacro.h" 1
		msr basepri, r3	
@ 0 "" 2
.LVL8:
	.thumb
.L18:
	bx	lr
.L21:
	.align	2
.L20:
	.word	.LANCHOR0
.LBE21:
.LBE20:
	.cfi_endproc
.LFE12:
	.size	vPortExitCritical, .-vPortExitCritical
	.section	.text.xPortPendSVHandler,"ax",%progbits
	.align	2
	.global	xPortPendSVHandler
	.thumb
	.thumb_func
	.type	xPortPendSVHandler, %function
xPortPendSVHandler:
.LFB13:
	.loc 1 438 0
	.cfi_startproc
	@ Naked Function: prologue and epilogue provided by programmer.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	.loc 1 441 0
@ 441 "port.c" 1
		mrs r0, psp							
	isb									
										
	ldr	r3, pxCurrentTCBConst			
	ldr	r2, [r3]						
										
	tst r14, #0x10						
	it eq								
	vstmdbeq r0!, {s16-s31}				
										
	stmdb r0!, {r4-r11, r14}			
	str r0, [r2]						
										
	stmdb sp!, {r0, r3}					
	mov r0, #80 							
	msr basepri, r0						
	dsb									
	isb									
	bl vTaskSwitchContext				
	mov r0, #0							
	msr basepri, r0						
	ldmia sp!, {r0, r3}					
										
	ldr r1, [r3]						
	ldr r0, [r1]						
										
	ldmia r0!, {r4-r11, r14}			
										
	tst r14, #0x10						
	it eq								
	vldmiaeq r0!, {s16-s31}				
										
	msr psp, r0							
	isb									
										
										
	bx r14								
										
	.align 4							
pxCurrentTCBConst: .word pxCurrentTCB	

@ 0 "" 2
	.loc 1 491 0
	.thumb
	.cfi_endproc
.LFE13:
	.size	xPortPendSVHandler, .-xPortPendSVHandler
	.section	.text.xPortSysTickHandler,"ax",%progbits
	.align	2
	.global	xPortSysTickHandler
	.thumb
	.thumb_func
	.type	xPortSysTickHandler, %function
xPortSysTickHandler:
.LFB14:
	.loc 1 495 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, lr}
	.cfi_def_cfa_offset 8
	.cfi_offset 3, -8
	.cfi_offset 14, -4
.LBB22:
.LBB23:
	.loc 2 195 0
@ 195 "./portmacro.h" 1
		mov r3, #80												
	msr basepri, r3											
	isb														
	dsb														

@ 0 "" 2
.LVL9:
	.thumb
.LBE23:
.LBE22:
	.loc 1 503 0
	bl	xTaskIncrementTick
.LVL10:
	cbz	r0, .L24
	.loc 1 507 0
	ldr	r3, .L29
	mov	r2, #268435456
	str	r2, [r3]
.L24:
.LVL11:
.LBB24:
.LBB25:
	.loc 2 229 0
	movs	r3, #0
@ 229 "./portmacro.h" 1
		msr basepri, r3	
@ 0 "" 2
	.thumb
	pop	{r3, pc}
.L30:
	.align	2
.L29:
	.word	-536810236
.LBE25:
.LBE24:
	.cfi_endproc
.LFE14:
	.size	xPortSysTickHandler, .-xPortSysTickHandler
	.section	.text.vPortSuppressTicksAndSleep,"ax",%progbits
	.align	2
	.weak	vPortSuppressTicksAndSleep
	.thumb
	.thumb_func
	.type	vPortSuppressTicksAndSleep, %function
vPortSuppressTicksAndSleep:
.LFB15:
	.loc 1 517 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
.LVL12:
	push	{r4, r5, r6, lr}
	.cfi_def_cfa_offset 16
	.cfi_offset 4, -16
	.cfi_offset 5, -12
	.cfi_offset 6, -8
	.cfi_offset 14, -4
	.loc 1 522 0
	ldr	r3, .L45
	.loc 1 531 0
	ldr	r2, .L45+4
	.loc 1 522 0
	ldr	r3, [r3]
	.loc 1 536 0
	ldr	r5, .L45+8
	ldr	r1, .L45+12
	.loc 1 537 0
	ldr	r6, .L45+16
	.loc 1 517 0
	sub	sp, sp, #16
	.cfi_def_cfa_offset 32
	.loc 1 522 0
	cmp	r0, r3
	.loc 1 517 0
	str	r0, [sp, #4]
	.loc 1 524 0
	it	hi
	strhi	r3, [sp, #4]
	.loc 1 531 0
	ldr	r3, [r2]
	bic	r3, r3, #1
	str	r3, [r2]
	.loc 1 536 0
	ldr	r3, [sp, #4]
	ldr	r1, [r1]
	ldr	r4, [r5]
	.loc 1 537 0
	ldr	r2, [r6]
	.loc 1 536 0
	subs	r3, r3, #1
	mla	r4, r4, r3, r1
.LVL13:
	.loc 1 537 0
	cmp	r4, r2
	.loc 1 539 0
	it	hi
	subhi	r4, r4, r2
.LVL14:
	.loc 1 544 0
@ 544 "port.c" 1
	cpsid i
@ 0 "" 2
	.loc 1 545 0
@ 545 "port.c" 1
	dsb
@ 0 "" 2
	.loc 1 546 0
@ 546 "port.c" 1
	isb
@ 0 "" 2
	.loc 1 550 0
	.thumb
	bl	eTaskConfirmSleepModeStatus
.LVL15:
	cbnz	r0, .L34
	.loc 1 554 0
	ldr	r3, .L45+12
	ldr	r2, .L45+20
	ldr	r3, [r3]
	.loc 1 557 0
	ldr	r1, .L45+4
	.loc 1 554 0
	str	r3, [r2]
	.loc 1 557 0
	ldr	r3, [r1]
	orr	r3, r3, #1
	str	r3, [r1]
	.loc 1 561 0
	ldr	r3, [r5]
	subs	r3, r3, #1
	str	r3, [r2]
	.loc 1 565 0
@ 565 "port.c" 1
	cpsie i
@ 0 "" 2
	.loc 1 676 0
	.thumb
	add	sp, sp, #16
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	@ sp needed
	pop	{r4, r5, r6, pc}
.LVL16:
.L34:
	.cfi_restore_state
	.loc 1 570 0
	ldr	r1, .L45+20
	.loc 1 574 0
	ldr	r3, .L45+12
	.loc 1 570 0
	str	r4, [r1]
	.loc 1 577 0
	ldr	r2, .L45+4
	.loc 1 574 0
	movs	r1, #0
	str	r1, [r3]
	.loc 1 577 0
	ldr	r3, [r2]
	orr	r3, r3, #1
	str	r3, [r2]
	.loc 1 584 0
	add	r0, sp, #16
	ldr	r3, [sp, #4]
	str	r3, [r0, #-4]!
	.loc 1 585 0
	bl	freertos_pre_sleep_processing
.LVL17:
	.loc 1 586 0
	ldr	r3, [sp, #12]
	cbz	r3, .L36
	.loc 1 588 0
@ 588 "port.c" 1
	dsb
@ 0 "" 2
	.loc 1 589 0
@ 589 "port.c" 1
	wfi
@ 0 "" 2
	.loc 1 590 0
@ 590 "port.c" 1
	isb
@ 0 "" 2
	.thumb
.L36:
	.loc 1 592 0
	add	r0, sp, #4
	bl	freertos_post_sleep_processing
.LVL18:
	.loc 1 597 0
@ 597 "port.c" 1
	cpsie i
@ 0 "" 2
	.loc 1 598 0
@ 598 "port.c" 1
	dsb
@ 0 "" 2
	.loc 1 599 0
@ 599 "port.c" 1
	isb
@ 0 "" 2
	.loc 1 605 0
@ 605 "port.c" 1
	cpsid i
@ 0 "" 2
	.loc 1 606 0
@ 606 "port.c" 1
	dsb
@ 0 "" 2
	.loc 1 607 0
@ 607 "port.c" 1
	isb
@ 0 "" 2
	.loc 1 616 0
	.thumb
	ldr	r3, .L45+4
	movs	r2, #2
	str	r2, [r3]
	.loc 1 623 0
	ldr	r3, [r3]
	lsls	r3, r3, #15
	bpl	.L37
.LBB26:
	.loc 1 631 0
	ldr	r3, .L45+12
	ldr	r2, [r5]
	ldr	r3, [r3]
	.loc 1 636 0
	ldr	r1, [r6]
	add	r3, r3, r2
	subs	r3, r3, #1
	.loc 1 631 0
	subs	r4, r3, r4
.LVL19:
	.loc 1 636 0
	cmp	r4, r1
	bcc	.L38
	.loc 1 636 0 is_stmt 0 discriminator 1
	cmp	r2, r4
	bcs	.L39
.L38:
	.loc 1 638 0 is_stmt 1
	subs	r4, r2, #1
.LVL20:
.L39:
	.loc 1 641 0
	ldr	r3, .L45+20
	str	r4, [r3]
	.loc 1 646 0
	ldr	r0, [sp, #4]
	subs	r0, r0, #1
.LVL21:
.LBE26:
	b	.L40
.LVL22:
.L37:
	.loc 1 654 0
	ldr	r2, .L45+12
	ldr	r3, [r5]
	ldr	r1, [sp, #4]
	ldr	r2, [r2]
	.loc 1 662 0
	ldr	r4, .L45+20
.LVL23:
	.loc 1 654 0
	mul	r1, r1, r3
	subs	r2, r1, r2
.LVL24:
	.loc 1 658 0
	udiv	r0, r2, r3
.LVL25:
	.loc 1 662 0
	mla	r3, r0, r3, r3
	subs	r3, r3, r2
	str	r3, [r4]
.LVL26:
.L40:
	.loc 1 668 0
	ldr	r3, .L45+12
	.loc 1 669 0
	ldr	r2, .L45+4
	.loc 1 668 0
	movs	r1, #0
	str	r1, [r3]
	.loc 1 669 0
	ldr	r3, [r2]
	orr	r3, r3, #1
	str	r3, [r2]
	.loc 1 670 0
	bl	vTaskStepTick
.LVL27:
	.loc 1 671 0
	ldr	r3, [r5]
	ldr	r2, .L45+20
	subs	r3, r3, #1
	str	r3, [r2]
	.loc 1 674 0
@ 674 "port.c" 1
	cpsie i
@ 0 "" 2
	.loc 1 676 0
	.thumb
	add	sp, sp, #16
	.cfi_def_cfa_offset 16
	@ sp needed
	pop	{r4, r5, r6, pc}
.L46:
	.align	2
.L45:
	.word	.LANCHOR1
	.word	-536813552
	.word	.LANCHOR2
	.word	-536813544
	.word	.LANCHOR3
	.word	-536813548
	.cfi_endproc
.LFE15:
	.size	vPortSuppressTicksAndSleep, .-vPortSuppressTicksAndSleep
	.section	.text.vPortSetupTimerInterrupt,"ax",%progbits
	.align	2
	.weak	vPortSetupTimerInterrupt
	.thumb
	.thumb_func
	.type	vPortSetupTimerInterrupt, %function
vPortSetupTimerInterrupt:
.LFB16:
	.loc 1 686 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, lr}
	.cfi_def_cfa_offset 20
	.cfi_offset 4, -20
	.cfi_offset 5, -16
	.cfi_offset 6, -12
	.cfi_offset 7, -8
	.cfi_offset 14, -4
	.loc 1 691 0
	ldr	r3, .L49
	.loc 1 697 0
	ldr	r2, .L49+4
	.loc 1 691 0
	ldr	r1, .L49+8
	.loc 1 690 0
	ldr	r7, .L49+12
	.loc 1 692 0
	ldr	r6, .L49+16
	.loc 1 698 0
	ldr	r5, .L49+20
	.loc 1 701 0
	ldr	r0, .L49+24
	.loc 1 691 0
	str	r1, [r3]
	.loc 1 690 0
	mov	lr, #32
	.loc 1 692 0
	movs	r3, #0
	.loc 1 701 0
	movs	r4, #31
	.loc 1 702 0
	movs	r1, #3
	.loc 1 690 0
	str	lr, [r7]
	.loc 1 692 0
	str	r3, [r6]
	.loc 1 697 0
	str	r3, [r2]
	.loc 1 698 0
	str	r3, [r5]
	.loc 1 701 0
	str	r4, [r0]
	.loc 1 702 0
	str	r1, [r2]
	pop	{r4, r5, r6, r7, pc}
.L50:
	.align	2
.L49:
	.word	.LANCHOR1
	.word	-536813552
	.word	524287
	.word	.LANCHOR2
	.word	.LANCHOR3
	.word	-536813544
	.word	-536813548
	.cfi_endproc
.LFE16:
	.size	vPortSetupTimerInterrupt, .-vPortSetupTimerInterrupt
	.section	.text.xPortStartScheduler,"ax",%progbits
	.align	2
	.global	xPortStartScheduler
	.thumb
	.thumb_func
	.type	xPortStartScheduler, %function
xPortStartScheduler:
.LFB9:
	.loc 1 288 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	.loc 1 368 0
	ldr	r3, .L56
	ldr	r2, [r3]
	.loc 1 288 0
	push	{r4, lr}
	.cfi_def_cfa_offset 8
	.cfi_offset 4, -8
	.cfi_offset 14, -4
	.loc 1 368 0
	orr	r2, r2, #15728640
	str	r2, [r3]
	.loc 1 369 0
	ldr	r2, [r3]
	orr	r2, r2, #-268435456
	.loc 1 288 0
	sub	sp, sp, #8
	.cfi_def_cfa_offset 16
	.loc 1 369 0
	str	r2, [r3]
	.loc 1 373 0
	bl	vPortSetupTimerInterrupt
.LVL28:
	.loc 1 376 0
	ldr	r3, .L56+4
	movs	r4, #0
	str	r4, [r3]
	.loc 1 379 0
	bl	vPortEnableVFP
.LVL29:
	.loc 1 382 0
	ldr	r2, .L56+8
	ldr	r3, [r2]
	orr	r3, r3, #-1073741824
	str	r3, [r2]
	.loc 1 385 0
	bl	prvPortStartFirstTask
.LVL30:
	.loc 1 393 0
	bl	vTaskSwitchContext
.LVL31:
.LBB27:
.LBB28:
	.loc 1 219 0
	str	r4, [sp, #4]
.LBB29:
.LBB30:
	.loc 2 195 0
@ 195 "./portmacro.h" 1
		mov r3, #80												
	msr basepri, r3											
	isb														
	dsb														

@ 0 "" 2
.LVL32:
	.thumb
.L52:
.LBE30:
.LBE29:
	.loc 1 229 0
	ldr	r3, [sp, #4]
	cmp	r3, #0
	beq	.L52
.LBE28:
.LBE27:
	.loc 1 398 0
	movs	r0, #0
	add	sp, sp, #8
	.cfi_def_cfa_offset 8
	@ sp needed
	pop	{r4, pc}
.L57:
	.align	2
.L56:
	.word	-536810208
	.word	.LANCHOR0
	.word	-536809676
	.cfi_endproc
.LFE9:
	.size	xPortStartScheduler, .-xPortStartScheduler
	.section	.text.vApplicationIdleHook,"ax",%progbits
	.align	2
	.global	vApplicationIdleHook
	.thumb
	.thumb_func
	.type	vApplicationIdleHook, %function
vApplicationIdleHook:
.LFB18:
	.loc 1 783 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	bx	lr
	.cfi_endproc
.LFE18:
	.size	vApplicationIdleHook, .-vApplicationIdleHook
	.section	.bss.ulTimerCountsForOneTick,"aw",%nobits
	.align	2
	.set	.LANCHOR2,. + 0
	.type	ulTimerCountsForOneTick, %object
	.size	ulTimerCountsForOneTick, 4
ulTimerCountsForOneTick:
	.space	4
	.section	.bss.ulStoppedTimerCompensation,"aw",%nobits
	.align	2
	.set	.LANCHOR3,. + 0
	.type	ulStoppedTimerCompensation, %object
	.size	ulStoppedTimerCompensation, 4
ulStoppedTimerCompensation:
	.space	4
	.section	.bss.xMaximumPossibleSuppressedTicks,"aw",%nobits
	.align	2
	.set	.LANCHOR1,. + 0
	.type	xMaximumPossibleSuppressedTicks, %object
	.size	xMaximumPossibleSuppressedTicks, 4
xMaximumPossibleSuppressedTicks:
	.space	4
	.section	.data.uxCriticalNesting,"aw",%progbits
	.align	2
	.set	.LANCHOR0,. + 0
	.type	uxCriticalNesting, %object
	.size	uxCriticalNesting, 4
uxCriticalNesting:
	.word	-1431655766
	.text
.Letext0:
	.file 3 "/export/disk2/qt/v4.1a.mp/project/weifateck_bellaz_va0_gcc_verification/toolchain/linux/asdk-4.9.3-m4-EL-rtos-n2.1-a16ft-150519/include/machine/_default_types.h"
	.file 4 "/export/disk2/qt/v4.1a.mp/project/weifateck_bellaz_va0_gcc_verification/toolchain/linux/asdk-4.9.3-m4-EL-rtos-n2.1-a16ft-150519/include/stdint.h"
	.file 5 "/export/disk2/qt/v4.1a.mp/project/weifateck_bellaz_va0_gcc_verification/asdk/../../../component/os/freertos/freertos_v10.0.1/Source/include/projdefs.h"
	.file 6 "/export/disk2/qt/v4.1a.mp/project/weifateck_bellaz_va0_gcc_verification/asdk/../../../component/os/freertos/freertos_v10.0.1/Source/include/task.h"
	.file 7 "/export/disk2/qt/v4.1a.mp/project/weifateck_bellaz_va0_gcc_verification/asdk/../inc/FreeRTOSConfig.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x539
	.2byte	0x3
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF58
	.byte	0x1
	.4byte	.LASF59
	.4byte	.LASF60
	.4byte	.Ldebug_ranges0+0
	.4byte	0
	.4byte	0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.4byte	.LASF1
	.uleb128 0x3
	.byte	0x1
	.byte	0x8
	.4byte	.LASF2
	.uleb128 0x3
	.byte	0x2
	.byte	0x5
	.4byte	.LASF3
	.uleb128 0x3
	.byte	0x2
	.byte	0x7
	.4byte	.LASF4
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x4
	.4byte	.LASF9
	.byte	0x3
	.byte	0x41
	.4byte	0x65
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.4byte	.LASF6
	.uleb128 0x3
	.byte	0x8
	.byte	0x5
	.4byte	.LASF7
	.uleb128 0x3
	.byte	0x8
	.byte	0x7
	.4byte	.LASF8
	.uleb128 0x4
	.4byte	.LASF10
	.byte	0x4
	.byte	0x42
	.4byte	0x5a
	.uleb128 0x4
	.4byte	.LASF11
	.byte	0x5
	.byte	0x23
	.4byte	0x90
	.uleb128 0x5
	.byte	0x4
	.4byte	0x96
	.uleb128 0x6
	.byte	0x1
	.4byte	0xa2
	.uleb128 0x7
	.4byte	0xa2
	.byte	0
	.uleb128 0x8
	.byte	0x4
	.uleb128 0x4
	.4byte	.LASF12
	.byte	0x2
	.byte	0x37
	.4byte	0x7a
	.uleb128 0x4
	.4byte	.LASF13
	.byte	0x2
	.byte	0x38
	.4byte	0x53
	.uleb128 0x4
	.4byte	.LASF14
	.byte	0x2
	.byte	0x39
	.4byte	0x65
	.uleb128 0x4
	.4byte	.LASF15
	.byte	0x2
	.byte	0x3f
	.4byte	0x7a
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.4byte	.LASF16
	.uleb128 0x3
	.byte	0x1
	.byte	0x8
	.4byte	.LASF17
	.uleb128 0x5
	.byte	0x4
	.4byte	0xa4
	.uleb128 0x9
	.byte	0x4
	.byte	0x6
	.byte	0x8f
	.4byte	0xff
	.uleb128 0xa
	.4byte	.LASF18
	.sleb128 0
	.uleb128 0xa
	.4byte	.LASF19
	.sleb128 1
	.uleb128 0xa
	.4byte	.LASF20
	.sleb128 2
	.byte	0
	.uleb128 0x4
	.4byte	.LASF21
	.byte	0x6
	.byte	0x93
	.4byte	0xe4
	.uleb128 0xb
	.4byte	.LASF22
	.byte	0x2
	.byte	0xbf
	.byte	0x1
	.byte	0x3
	.4byte	0x123
	.uleb128 0xc
	.4byte	.LASF25
	.byte	0x2
	.byte	0xc1
	.4byte	0x7a
	.byte	0
	.uleb128 0xb
	.4byte	.LASF23
	.byte	0x2
	.byte	0xe3
	.byte	0x1
	.byte	0x3
	.4byte	0x13c
	.uleb128 0xd
	.4byte	.LASF61
	.byte	0x2
	.byte	0xe3
	.4byte	0x7a
	.byte	0
	.uleb128 0xb
	.4byte	.LASF24
	.byte	0x1
	.byte	0xd9
	.byte	0x1
	.byte	0x1
	.4byte	0x155
	.uleb128 0xc
	.4byte	.LASF26
	.byte	0x1
	.byte	0xdb
	.4byte	0x155
	.byte	0
	.uleb128 0xe
	.4byte	0x7a
	.uleb128 0xf
	.4byte	0x13c
	.4byte	.LFB6
	.4byte	.LFE6
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.4byte	0x196
	.uleb128 0x10
	.4byte	0x149
	.byte	0x2
	.byte	0x91
	.sleb128 -4
	.uleb128 0x11
	.4byte	0x10a
	.4byte	.LBB16
	.4byte	.LBE16
	.byte	0x1
	.byte	0xe4
	.uleb128 0x12
	.4byte	.LBB17
	.4byte	.LBE17
	.uleb128 0x13
	.4byte	0x117
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x14
	.4byte	.LASF27
	.byte	0x1
	.2byte	0x105
	.byte	0x1
	.4byte	.LFB8
	.4byte	.LFE8
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x14
	.4byte	.LASF28
	.byte	0x1
	.2byte	0x2c3
	.byte	0x1
	.4byte	.LFB17
	.4byte	.LFE17
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x15
	.byte	0x1
	.4byte	.LASF46
	.byte	0x1
	.byte	0xbb
	.byte	0x1
	.4byte	0xde
	.4byte	.LFB5
	.4byte	.LFE5
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.4byte	0x206
	.uleb128 0x16
	.4byte	.LASF29
	.byte	0x1
	.byte	0xbb
	.4byte	0xde
	.4byte	.LLST0
	.uleb128 0x16
	.4byte	.LASF30
	.byte	0x1
	.byte	0xbb
	.4byte	0x85
	.4byte	.LLST1
	.uleb128 0x17
	.4byte	.LASF31
	.byte	0x1
	.byte	0xbb
	.4byte	0xa2
	.byte	0x1
	.byte	0x52
	.byte	0
	.uleb128 0x18
	.byte	0x1
	.4byte	.LASF32
	.byte	0x1
	.byte	0xf2
	.byte	0x1
	.4byte	.LFB7
	.4byte	.LFE7
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF33
	.byte	0x1
	.2byte	0x191
	.byte	0x1
	.4byte	.LFB10
	.4byte	.LFE10
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF34
	.byte	0x1
	.2byte	0x199
	.byte	0x1
	.4byte	.LFB11
	.4byte	.LFE11
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.4byte	0x269
	.uleb128 0x1b
	.4byte	0x10a
	.4byte	.LBB18
	.4byte	.LBE18
	.byte	0x1
	.2byte	0x19b
	.uleb128 0x12
	.4byte	.LBB19
	.4byte	.LBE19
	.uleb128 0x13
	.4byte	0x117
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF35
	.byte	0x1
	.2byte	0x1aa
	.byte	0x1
	.4byte	.LFB12
	.4byte	.LFE12
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.4byte	0x29d
	.uleb128 0x1b
	.4byte	0x123
	.4byte	.LBB20
	.4byte	.LBE20
	.byte	0x1
	.2byte	0x1b0
	.uleb128 0x1c
	.4byte	0x130
	.4byte	.LLST2
	.byte	0
	.byte	0
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF36
	.byte	0x1
	.2byte	0x1b5
	.byte	0x1
	.4byte	.LFB13
	.4byte	.LFE13
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF37
	.byte	0x1
	.2byte	0x1ee
	.byte	0x1
	.4byte	.LFB14
	.4byte	.LFE14
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.4byte	0x314
	.uleb128 0x1d
	.4byte	0x10a
	.4byte	.LBB22
	.4byte	.LBE22
	.byte	0x1
	.2byte	0x1f4
	.4byte	0x2ef
	.uleb128 0x12
	.4byte	.LBB23
	.4byte	.LBE23
	.uleb128 0x13
	.4byte	0x117
	.byte	0
	.byte	0
	.uleb128 0x1d
	.4byte	0x123
	.4byte	.LBB24
	.4byte	.LBE24
	.byte	0x1
	.2byte	0x1fe
	.4byte	0x30a
	.uleb128 0x1e
	.4byte	0x130
	.byte	0
	.byte	0
	.uleb128 0x1f
	.4byte	.LVL10
	.4byte	0x4cb
	.byte	0
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF38
	.byte	0x1
	.2byte	0x204
	.byte	0x1
	.4byte	.LFB15
	.4byte	.LFE15
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.4byte	0x3d5
	.uleb128 0x20
	.4byte	.LASF39
	.byte	0x1
	.2byte	0x204
	.4byte	0xc5
	.4byte	.LLST3
	.uleb128 0x21
	.4byte	.LASF40
	.byte	0x1
	.2byte	0x206
	.4byte	0x7a
	.4byte	.LLST4
	.uleb128 0x21
	.4byte	.LASF41
	.byte	0x1
	.2byte	0x206
	.4byte	0x7a
	.4byte	.LLST5
	.uleb128 0x21
	.4byte	.LASF42
	.byte	0x1
	.2byte	0x206
	.4byte	0x7a
	.4byte	.LLST6
	.uleb128 0x22
	.4byte	.LASF43
	.byte	0x1
	.2byte	0x207
	.4byte	0xc5
	.byte	0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x23
	.4byte	.LBB26
	.4byte	.LBE26
	.4byte	0x39a
	.uleb128 0x21
	.4byte	.LASF44
	.byte	0x1
	.2byte	0x271
	.4byte	0x7a
	.4byte	.LLST7
	.byte	0
	.uleb128 0x1f
	.4byte	.LVL15
	.4byte	0x4da
	.uleb128 0x24
	.4byte	.LVL17
	.4byte	0x4e9
	.4byte	0x3b7
	.uleb128 0x25
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x91
	.sleb128 -20
	.byte	0
	.uleb128 0x24
	.4byte	.LVL18
	.4byte	0x503
	.4byte	0x3cb
	.uleb128 0x25
	.byte	0x1
	.byte	0x50
	.byte	0x2
	.byte	0x91
	.sleb128 -28
	.byte	0
	.uleb128 0x1f
	.4byte	.LVL27
	.4byte	0x517
	.byte	0
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF45
	.byte	0x1
	.2byte	0x2ad
	.byte	0x1
	.4byte	.LFB16
	.4byte	.LFE16
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x26
	.byte	0x1
	.4byte	.LASF47
	.byte	0x1
	.2byte	0x11f
	.byte	0x1
	.4byte	0xaf
	.4byte	.LFB9
	.4byte	.LFE9
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.4byte	0x472
	.uleb128 0x1d
	.4byte	0x13c
	.4byte	.LBB27
	.4byte	.LBE27
	.byte	0x1
	.2byte	0x18a
	.4byte	0x44d
	.uleb128 0x12
	.4byte	.LBB28
	.4byte	.LBE28
	.uleb128 0x10
	.4byte	0x149
	.byte	0x2
	.byte	0x91
	.sleb128 -12
	.uleb128 0x11
	.4byte	0x10a
	.4byte	.LBB29
	.4byte	.LBE29
	.byte	0x1
	.byte	0xe4
	.uleb128 0x12
	.4byte	.LBB30
	.4byte	.LBE30
	.uleb128 0x13
	.4byte	0x117
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1f
	.4byte	.LVL28
	.4byte	0x3d5
	.uleb128 0x1f
	.4byte	.LVL29
	.4byte	0x1aa
	.uleb128 0x1f
	.4byte	.LVL30
	.4byte	0x196
	.uleb128 0x1f
	.4byte	.LVL31
	.4byte	0x531
	.byte	0
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF48
	.byte	0x1
	.2byte	0x30e
	.byte	0x1
	.4byte	.LFB18
	.4byte	.LFE18
	.byte	0x1
	.byte	0x9c
	.byte	0x1
	.uleb128 0x27
	.4byte	.LASF49
	.byte	0x1
	.byte	0x92
	.4byte	0xba
	.byte	0x5
	.byte	0x3
	.4byte	uxCriticalNesting
	.uleb128 0x27
	.4byte	.LASF50
	.byte	0x1
	.byte	0x98
	.4byte	0x7a
	.byte	0x5
	.byte	0x3
	.4byte	ulTimerCountsForOneTick
	.uleb128 0x27
	.4byte	.LASF51
	.byte	0x1
	.byte	0xa0
	.4byte	0x7a
	.byte	0x5
	.byte	0x3
	.4byte	xMaximumPossibleSuppressedTicks
	.uleb128 0x27
	.4byte	.LASF52
	.byte	0x1
	.byte	0xa8
	.4byte	0x7a
	.byte	0x5
	.byte	0x3
	.4byte	ulStoppedTimerCompensation
	.uleb128 0x28
	.byte	0x1
	.4byte	.LASF53
	.byte	0x6
	.2byte	0x875
	.byte	0x1
	.4byte	0xaf
	.byte	0x1
	.uleb128 0x28
	.byte	0x1
	.4byte	.LASF54
	.byte	0x6
	.2byte	0x92c
	.byte	0x1
	.4byte	0xff
	.byte	0x1
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF55
	.byte	0x7
	.byte	0x8a
	.byte	0x1
	.byte	0x1
	.4byte	0x4fd
	.uleb128 0x7
	.4byte	0x4fd
	.byte	0
	.uleb128 0x5
	.byte	0x4
	.4byte	0x30
	.uleb128 0x29
	.byte	0x1
	.4byte	.LASF56
	.byte	0x7
	.byte	0x8b
	.byte	0x1
	.byte	0x1
	.4byte	0x517
	.uleb128 0x7
	.4byte	0x4fd
	.byte	0
	.uleb128 0x2a
	.byte	0x1
	.4byte	.LASF57
	.byte	0x6
	.2byte	0x916
	.byte	0x1
	.byte	0x1
	.4byte	0x52c
	.uleb128 0x7
	.4byte	0x52c
	.byte	0
	.uleb128 0x2b
	.4byte	0xc5
	.uleb128 0x2c
	.byte	0x1
	.4byte	.LASF62
	.byte	0x6
	.2byte	0x8c9
	.byte	0x1
	.byte	0x1
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x4
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x20
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x35
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x34
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x58
	.uleb128 0xb
	.uleb128 0x59
	.uleb128 0x5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x5
	.byte	0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x1f
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x20
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x21
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x22
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x23
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x24
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x25
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0xa
	.uleb128 0x2111
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x26
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x27
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x28
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x29
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2a
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2b
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x2c
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LVL1
	.4byte	.LVL1
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL1
	.4byte	.LVL3
	.2byte	0x3
	.byte	0x70
	.sleb128 -4
	.byte	0x9f
	.4byte	.LVL3
	.4byte	.LVL4
	.2byte	0x3
	.byte	0x70
	.sleb128 -36
	.byte	0x9f
	.4byte	.LVL4
	.4byte	.LVL5
	.2byte	0x4
	.byte	0x70
	.sleb128 -68
	.byte	0x9f
	.4byte	.LVL5
	.4byte	.LFE5
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL1
	.4byte	.LVL2
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL2
	.4byte	.LFE5
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL7
	.4byte	.LVL8
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL12
	.4byte	.LVL15-1
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL15-1
	.4byte	.LFE15
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x50
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL13
	.4byte	.LVL19
	.2byte	0x1
	.byte	0x54
	.4byte	.LVL22
	.4byte	.LVL23
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL21
	.4byte	.LVL22
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL25
	.4byte	.LVL27-1
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LVL24
	.4byte	.LVL26
	.2byte	0x1
	.byte	0x52
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL19
	.4byte	.LVL22
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x84
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.LFB6
	.4byte	.LFE6-.LFB6
	.4byte	.LFB8
	.4byte	.LFE8-.LFB8
	.4byte	.LFB17
	.4byte	.LFE17-.LFB17
	.4byte	.LFB5
	.4byte	.LFE5-.LFB5
	.4byte	.LFB7
	.4byte	.LFE7-.LFB7
	.4byte	.LFB10
	.4byte	.LFE10-.LFB10
	.4byte	.LFB11
	.4byte	.LFE11-.LFB11
	.4byte	.LFB12
	.4byte	.LFE12-.LFB12
	.4byte	.LFB13
	.4byte	.LFE13-.LFB13
	.4byte	.LFB14
	.4byte	.LFE14-.LFB14
	.4byte	.LFB15
	.4byte	.LFE15-.LFB15
	.4byte	.LFB16
	.4byte	.LFE16-.LFB16
	.4byte	.LFB9
	.4byte	.LFE9-.LFB9
	.4byte	.LFB18
	.4byte	.LFE18-.LFB18
	.4byte	0
	.4byte	0
	.section	.debug_ranges,"",%progbits
.Ldebug_ranges0:
	.4byte	.LFB6
	.4byte	.LFE6
	.4byte	.LFB8
	.4byte	.LFE8
	.4byte	.LFB17
	.4byte	.LFE17
	.4byte	.LFB5
	.4byte	.LFE5
	.4byte	.LFB7
	.4byte	.LFE7
	.4byte	.LFB10
	.4byte	.LFE10
	.4byte	.LFB11
	.4byte	.LFE11
	.4byte	.LFB12
	.4byte	.LFE12
	.4byte	.LFB13
	.4byte	.LFE13
	.4byte	.LFB14
	.4byte	.LFE14
	.4byte	.LFB15
	.4byte	.LFE15
	.4byte	.LFB16
	.4byte	.LFE16
	.4byte	.LFB9
	.4byte	.LFE9
	.4byte	.LFB18
	.4byte	.LFE18
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF26:
	.ascii	"ulDummy\000"
.LASF61:
	.ascii	"ulNewMaskValue\000"
.LASF60:
	.ascii	"/export/disk2/qt/v4.1a.mp/component/os/freertos/fre"
	.ascii	"ertos_v10.0.1/Source/portable/GCC/ARM_CM4F\000"
.LASF25:
	.ascii	"ulNewBASEPRI\000"
.LASF8:
	.ascii	"long long unsigned int\000"
.LASF37:
	.ascii	"xPortSysTickHandler\000"
.LASF31:
	.ascii	"pvParameters\000"
.LASF54:
	.ascii	"eTaskConfirmSleepModeStatus\000"
.LASF39:
	.ascii	"xExpectedIdleTime\000"
.LASF7:
	.ascii	"long long int\000"
.LASF1:
	.ascii	"signed char\000"
.LASF41:
	.ascii	"ulCompleteTickPeriods\000"
.LASF56:
	.ascii	"freertos_post_sleep_processing\000"
.LASF38:
	.ascii	"vPortSuppressTicksAndSleep\000"
.LASF42:
	.ascii	"ulCompletedSysTickDecrements\000"
.LASF5:
	.ascii	"long int\000"
.LASF21:
	.ascii	"eSleepModeStatus\000"
.LASF36:
	.ascii	"xPortPendSVHandler\000"
.LASF13:
	.ascii	"BaseType_t\000"
.LASF9:
	.ascii	"__uint32_t\000"
.LASF53:
	.ascii	"xTaskIncrementTick\000"
.LASF0:
	.ascii	"unsigned int\000"
.LASF6:
	.ascii	"long unsigned int\000"
.LASF29:
	.ascii	"pxTopOfStack\000"
.LASF27:
	.ascii	"prvPortStartFirstTask\000"
.LASF57:
	.ascii	"vTaskStepTick\000"
.LASF19:
	.ascii	"eStandardSleep\000"
.LASF4:
	.ascii	"short unsigned int\000"
.LASF12:
	.ascii	"StackType_t\000"
.LASF52:
	.ascii	"ulStoppedTimerCompensation\000"
.LASF58:
	.ascii	"GNU C 4.9.3 20150311 (prerelease) -mfpu=fpv4-sp-d16"
	.ascii	" -mfloat-abi=softfp -march=armv7e-m -mcpu=cortex-m4"
	.ascii	" -mthumb -mabi=aapcs -g -gdwarf-3 -O2 -fuse-tls -fn"
	.ascii	"o-short-enums -ffunction-sections -fdata-sections\000"
.LASF22:
	.ascii	"vPortRaiseBASEPRI\000"
.LASF43:
	.ascii	"xModifiableIdleTime\000"
.LASF49:
	.ascii	"uxCriticalNesting\000"
.LASF62:
	.ascii	"vTaskSwitchContext\000"
.LASF16:
	.ascii	"sizetype\000"
.LASF15:
	.ascii	"TickType_t\000"
.LASF47:
	.ascii	"xPortStartScheduler\000"
.LASF51:
	.ascii	"xMaximumPossibleSuppressedTicks\000"
.LASF33:
	.ascii	"vPortEndScheduler\000"
.LASF23:
	.ascii	"vPortSetBASEPRI\000"
.LASF24:
	.ascii	"prvTaskExitError\000"
.LASF2:
	.ascii	"unsigned char\000"
.LASF45:
	.ascii	"vPortSetupTimerInterrupt\000"
.LASF28:
	.ascii	"vPortEnableVFP\000"
.LASF30:
	.ascii	"pxCode\000"
.LASF3:
	.ascii	"short int\000"
.LASF44:
	.ascii	"ulCalculatedLoadValue\000"
.LASF11:
	.ascii	"TaskFunction_t\000"
.LASF48:
	.ascii	"vApplicationIdleHook\000"
.LASF10:
	.ascii	"uint32_t\000"
.LASF32:
	.ascii	"vPortSVCHandler\000"
.LASF17:
	.ascii	"char\000"
.LASF35:
	.ascii	"vPortExitCritical\000"
.LASF55:
	.ascii	"freertos_pre_sleep_processing\000"
.LASF20:
	.ascii	"eNoTasksWaitingTimeout\000"
.LASF59:
	.ascii	"port.c\000"
.LASF40:
	.ascii	"ulReloadValue\000"
.LASF34:
	.ascii	"vPortEnterCritical\000"
.LASF46:
	.ascii	"pxPortInitialiseStack\000"
.LASF50:
	.ascii	"ulTimerCountsForOneTick\000"
.LASF18:
	.ascii	"eAbortSleep\000"
.LASF14:
	.ascii	"UBaseType_t\000"
	.ident	"GCC: (Realtek ASDK-4.9.3 Build 2163) 4.9.3 20150311 (prerelease)"
