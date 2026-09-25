#include <lpc214x.h>
#include "mini_project1_defines.h"
//#include "types.h"
#include "mini_project1.h"

//delay

void delay_us(u32 dlyus)
{
	for(dlyus*=12; dlyus>0; dlyus--);
}

void delay_ms(u32 dlyms)
{	for(dlyms*=12000; dlyms>0; dlyms--);
}

void delay_s(u32 dlys)
{
	for(dlys*=12000000; dlys>0; dlys--);
}


//timer delay

void init_timer0(void)
{
	//reset counter
	T0TCR = 1<<1;
	//stop on MR0
	T0MCR = 1<<2;
}

void tdelay_us(u32 us)
{
	//set point using MR0
	T0MR0 = us;
	//set prescalar value
	T0PR = 14;//how 14 --> (15MHz(PCLK)/1MHz(TCLK))-1 
											//	= (15,000,000/1,000,000)-1
											//	=	((15*10^6)/10^6)-1
											//	= 15-1 = 14 (both 10^6's are cancelled)
	//reset timer counter to zero
	T0TC = 0;
	//start counting
	T0TCR = 1<<0;
	while(T0MR0!=T0TC);
}

void tdelay_ms(u32 ms)
{
	T0MR0 = ms;
	T0PR = 14999;
	T0TC = 0;
	T0TCR = 1<<0;
	while(T0MR0 != T0TC);
}

void tdelay_s(u32 s)
{
	T0MR0 = s;
	T0PR = 14999999;//0X00E4E1BF(HEXA VALUE)
	T0TC = 0;
	T0TCR = 1<<0;
	//while(T0MR0 != T0TC);
}


//Lcd

void WRITE_LCD_CMD(u8 cmd)
{
	//perform write operation(rw=0)
	SCLRBIT(IOCLR0,LCD_RW);
	//select command register(rs=0)
	SCLRBIT(IOCLR0,LCD_RS);
	//write cmd on to the data pins
	WRITEBYTE(IOPIN0,LCD_DATA,cmd);
	//apply H to L pulse on EN
	SSETBIT(IOSET0,LCD_EN);
	delay_ms(1);
	SCLRBIT(IOCLR0,LCD_EN);
	//delay for all internal process
	delay_ms(1);
}

void Init_LCD(void){
	//cfg p0.8 t0 p0.15(lcd data pins) as output
	WRITEBYTE(IODIR0,LCD_DATA,0xFF);
	// cfg p0.16(rs),p0.17(rw) and p0.18(en) as output
	SETBIT(IODIR0,LCD_RS);
	SETBIT(IODIR0,LCD_RW);
	SETBIT(IODIR0,LCD_EN);
	
	delay_ms(15);
	WRITE_LCD_CMD(MODE_8BIT_1LINE);
	delay_ms(5);
	WRITE_LCD_CMD(MODE_8BIT_1LINE);
	delay_us(100);
	WRITE_LCD_CMD(MODE_8BIT_1LINE);
	
	WRITE_LCD_CMD(MODE_8BIT_2LINE);
	WRITE_LCD_CMD(DISP_ON_CUR_OFF);
	WRITE_LCD_CMD(CLEAR_LCD);
	WRITE_LCD_CMD(SHIFT_CUR_RIGHT);
}

void WRITE_LCD_DATA(u8 ascii)
{
	SCLRBIT(IOCLR0,LCD_RW);
	//select data register(rs=1)
	SSETBIT(IOSET0,LCD_RS);
	//write data on to the data pins
	WRITEBYTE(IOPIN0,LCD_DATA,ascii);
	//apply H to L pulse on EN
	SSETBIT(IOSET0,LCD_EN); // EN = 1
	delay_ms(1);
	SCLRBIT(IOCLR0,LCD_EN); // EN = 0
	//delay for internal process
	delay_ms(1);
}

void StrLCD(s8* str)
{
	while(*str)
	{
		WRITE_LCD_DATA(*str++);
	}
}

void BuildcgRAM1(u8* p, u8 c)
{
	s32 i;
	//select CGRAM
	WRITE_LCD_CMD(GOTO_CGRAM);
	for(i=0;i<c;i++)
	{
		WRITE_LCD_DATA(p[i]);
	}
	//select DDRAM
	//WRITE_LCD_CMD(GOTO_LINE1_POS0);
}

void u32LCD(u32 n)
{
	u8 a[10];
	s32 i=0;
	if(n==0)
	{
		WRITE_LCD_DATA('0');
	}
	while(n)
	{
		a[i++]=n%10+48;
		n/=10;
	}
	for(--i;i>=0;i--)
	{
		WRITE_LCD_DATA(a[i]);
	}
}


//kepypad

u8 kpmLUT[4][4] = {{'7','8','9','/'},
									{'4','5','6','*'},
									{'1','2','3','-'},
									{'c','0','=','+'}};

void Init_KPM(void){
	IODIR1|=15<<ROW0;
}

u32 colscan(void)
{
	if(((IOPIN1>>COL0)&15)<15)
		return 0;
	else
		return 1;
}

u32 rowcheck(void)
{
	u32 rno;
	for(rno=0;rno<4;rno++)
	{
		IOPIN1 = ((IOPIN1&~(15<<ROW0))|((~(1<<rno))<<ROW0));
		if(colscan()==0)
			break;
	}
	IOCLR1 = 15<<ROW0;
	return rno;
}

u32 colcheck(void)
{
	u32 cno;
	for(cno=0;cno<4;cno++)
	{
		if(((IOPIN1>>(COL0+cno))&1)==0)
			break;
	}
	return cno;
}

u32 keyscan(void)
{
	
	u32 rno,cno,key;
	//wait for switch press
	/*tdelay_s(10);
	while(colscan())
	{
		if(T0MR0 == T0TC)
				return 0;
	}*/
	while(colscan());
	
	//findout row number
	rno=rowcheck();
	
	//findout column number
	cno = colcheck();
	
	key = kpmLUT[rno][cno];
	//wait for switch release
	while(!(colscan()));
	//if(key >= '0' || key <= '9' || key == '-' || key == '=')
	return key;
}
	

//ADC


//ADC.c

void Init_ADC(void)
{
	//make p1.27 to p1.30 as GPIO
	PINSEL1 = (PINSEL1 & ~(0xff << ((27-16)*2)));
	
	//cfg p1.27 as AIN0
	PINSEL1 |= AD0_1|AD0_2;
	
	
	//cfgportpin(0,27,10);
	//PINSEL1 |= 0X15400000;
	AD0CR = 1 << PDN_BIT | CLKDIV_VALUE << CLKDIV;
}

void Read_ADC(u32 chno, u32* dval, f32* eAR)
{
	//clear previous channel values
	AD0CR &=~(255<<0);
	
	//select channel and start conversion
	AD0CR |= 1<<chno|1<<START_CONV;
	
	//wait for 3usec
	delay_us(3);
	//ADCR &= ~(1<<START_CONV);
	//check the done bit status
	while(((AD0GDR >> DONE_BIT)&1)==0);
	AD0CR &= ~(1<<START_CONV);
	
	//extract 10bit digital output
	*dval = ((AD0GDR>>RESULT)&1023);
	
	//find eAR value
	*eAR = ((3.3/1023)*(*dval));
}

u32 LM35TempC(void)
{
	u32 dval;
	f32 eAR;
	Read_ADC(1,&dval,&eAR);
	return(eAR*100);
}	

u32 MQ2GasLevel_ppm(void)
{
    u32 dval;
    f32 eAR;
    Read_ADC(2, &dval, &eAR);
   
		return 1023-dval;
    //return (dval * (10000-200))/1023 + 200;
}
