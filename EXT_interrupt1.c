#include<lpc214x.h>
#include "mini_project1.h"
#include "mini_project1_defines.h"

u32 flag;
void Eint_isr(void) __irq
{
				flag = 1;
				VICVectAddr = 0;
				EXTINT = 1<<0;
}
void Eint_Enable(void)
{
        //cfg p0.1 as fun4 (Eint0)
        PINSEL0 = 3<<(1*2);
        //select exint0 as irq
        VICIntSelect = 0<<Eint_CH;
        //enable exint0 as source
        VICIntEnable = 1<<Eint_CH;
        //load the isr address
        VICVectAddr0 = (u32)Eint_isr;
        //load the slot for exint0
        VICVectCntl0 = 1<<5|Eint_CH;
        //select edge triggering
        EXTMODE = 1<<0;
}
