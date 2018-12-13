 .section .vecbase.text, "x"
 .org 0

 .global VecBase
 .global InitVecBase
 .extern timer0_isr_handler
 .extern i2s_int
 .extern gpio_int

 .global DebugExceptionVector
 .global NMIExceptionVector
 .global KernelExceptionVector
 .global UserExceptionVector
 .global DoubleExceptionVector
 .global Level1Handler
 .global UserExceptionHandler

 .set BIT_TIM,  6
 .set BIT_GPIO, 4
 .set BIT_I2S,  1

 .set ENABLED_INTS_TIM,    0
 .set ENABLED_INTS_I2S,   (1 << BIT_TIM)
 .set ENABLED_INTS_GPIO, ((1 << BIT_TIM) | (1 << BIT_I2S))

VecBase:

    /* IMPORTANT: exception vector literals will go here, but we
     * can't have more than 4 otherwise we push DebugExceptionVector past
     * offset 0x10 relative to VecBase. There should be ways to avoid this,
     * and also keep the VecBase offsets easy to read, but this works for
     * now. */

 .literal_position
 .org VecBase + 0x10

DebugExceptionVector:
_M0AUTO:
/* { */
    waiti 2
    j _M0AUTO
/* } */

 .org VecBase + 0x20

NMIExceptionVector:
/* { */
    rfi 3
/* } */

 .org VecBase + 0x30

KernelExceptionVector:
_M1AUTO:
/* { */
    break 1, 0
    j _M1AUTO
/* } */

 .org VecBase + 0x50

UserExceptionVector:
/* { */
    addi sp,sp,-0x40
    s32i a12,sp,0		// a12

    rsr.exccause a12

    beqi a12,4,Level1Handler

	j UserExceptionHandler
/* } */

 .org VecBase + 0x70

DoubleExceptionVector:
_M2AUTO:
/* { */
    break 1, 4
    j _M2AUTO
/* } */

/*
	Что нужно сохранить:
    a2-a4 были сохранены ранее
	a0,a5-a15
	sar
	epc1
	ps
	intenable
*/

 .org VecBase + 0x80

 .literal_position

 .balign  16

Level1Handler:
/* { */
    s32i     a2,sp,4             // a2
    movi     a12,0x21
    xsr.ps   a12
    rsr.epc1 a2
    s32i     a12,sp,8            // ps
    s32i     a2, sp,12           // epc1

    rsr.intenable a12

_M4AUTO:
	/* { */
        rsr.interrupt a2

		and a2,a2,a12

        bbci a2,BIT_TIM,_M3AUTO
        /* { */
			// Прерывание таймера

	        rsr.sar  a2
	        s32i     a0, sp,16   // a0
	        s32i     a3, sp,20   // a3
	        s32i     a4, sp,24   // a4
	        s32i     a5, sp,28   // a5
	        s32i     a6, sp,32   // a6
	        s32i     a7, sp,36   // a7
	        s32i     a8, sp,40   // a8
	        s32i     a9, sp,44   // a9
	        s32i     a10,sp,48   // a10
	        s32i     a11,sp,52   // a11
	        s32i     a2, sp,56   // sar

			movi a2, 1 << BIT_TIM
	        wsr.intclear a2

			call0 timer0_isr_handler

	        l32i     a2, sp,56   // sar
	        l32i     a0, sp,16   // a0
	        l32i     a3, sp,20   // a3
	        l32i     a4, sp,24   // a4
	        l32i     a5, sp,28   // a5
	        l32i     a6, sp,32   // a6
	        l32i     a7, sp,36   // a7
	        l32i     a8, sp,40   // a8
	        l32i     a9, sp,44   // a9
	        l32i     a10,sp,48   // a10
	        l32i     a11,sp,52   // a11
	        wsr.sar  a2

            j     _M4AUTO
        /* } */
_M3AUTO:

        bbci a2,BIT_I2S,_M5AUTO
		/* { */
			// Прерывание I2S

	        movi a2,ENABLED_INTS_I2S
	        and  a2,a2,a12
        	wsr.intenable a2
    	    rsync
	        rsil     a2,0

	        rsr.sar  a2
	        s32i     a0, sp,16   // a0
	        s32i     a3, sp,20   // a3
	        s32i     a4, sp,24   // a4
	        s32i     a5, sp,28   // a5
	        s32i     a6, sp,32   // a6
	        s32i     a7, sp,36   // a7
	        s32i     a8, sp,40   // a8
	        s32i     a9, sp,44   // a9
	        s32i     a10,sp,48   // a10
	        s32i     a11,sp,52   // a11
	        s32i     a2, sp,56   // sar

			movi a2, 1 << BIT_I2S
	        wsr.intclear a2

			call0 i2s_int

	        l32i     a2, sp,56   // sar
	        l32i     a0, sp,16   // a0
	        l32i     a3, sp,20   // a3
	        l32i     a4, sp,24   // a4
	        l32i     a5, sp,28   // a5
	        l32i     a6, sp,32   // a6
	        l32i     a7, sp,36   // a7
	        l32i     a8, sp,40   // a8
	        l32i     a9, sp,44   // a9
	        l32i     a10,sp,48   // a10
	        l32i     a11,sp,52   // a11
	        wsr.sar  a2

	        rsil  a2,1
            j     _M4AUTO
		/* } */
_M5AUTO:

        bbci a2,BIT_GPIO,_M6AUTO
		/* { */
			// Прерывание GPIO

	        movi a2,ENABLED_INTS_GPIO
	        and  a2,a2,a12
        	wsr.intenable a2
    	    rsync
	        rsil     a2,0

	        rsr.sar  a2
	        s32i     a0, sp,16   // a0
	        s32i     a3, sp,20   // a3
	        s32i     a4, sp,24   // a4
	        s32i     a5, sp,28   // a5
	        s32i     a6, sp,32   // a6
	        s32i     a7, sp,36   // a7
	        s32i     a8, sp,40   // a8
	        s32i     a9, sp,44   // a9
	        s32i     a10,sp,48   // a10
	        s32i     a11,sp,52   // a11
	        s32i     a2, sp,56   // sar

			movi a2, 1 << BIT_GPIO
	        wsr.intclear a2

			call0 gpio_int

	        l32i     a2, sp,56   // sar
	        l32i     a0, sp,16   // a0
	        l32i     a3, sp,20   // a3
	        l32i     a4, sp,24   // a4
	        l32i     a5, sp,28   // a5
	        l32i     a6, sp,32   // a6
	        l32i     a7, sp,36   // a7
	        l32i     a8, sp,40   // a8
	        l32i     a9, sp,44   // a9
	        l32i     a10,sp,48   // a10
	        l32i     a11,sp,52   // a11
	        wsr.sar  a2

	        rsil  a2,1
            j     _M4AUTO
		/* } */
_M6AUTO:

        beqz a2,_M7AUTO

_M8AUTO:
		/* { */
	        break   1, 4
	        j       _M8AUTO
		/* } */
	/* } */
_M7AUTO:

   	wsr.intenable a12

    l32i     a12,sp,8            // ps
    l32i     a2, sp,12           // epc1

    wsr.ps   a12
    wsr.epc1 a2

    l32i     a12,sp,0            // a12
    l32i     a2, sp,4            // a2

    rsync

    addi    sp,sp,0x40

    rfe
/* } */

 .balign  4

UserExceptionHandler:
/* { */
    addi  sp,sp,-0x80
    addi  sp,sp,-0x40
//  addi  sp,sp,-0x100

    s32i  a2,sp,20
	s32i  a3,sp,24
    s32i  a4,sp,28
	mov   a2,a12
	l32i  a12,sp,0xC0

    movi  a3,0x3fffdaac
    l32i  a3,a3,0
    addx4 a3,a2,a3
    l32i  a3,a3,0

    jx    a3
/* } */

InitVecBase:
/* { */
	movi a2,VecBase
	wsr.vecbase a2

    rsync

	ret
/* } */
