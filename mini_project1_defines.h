//Defines

#define SETBIT(WORD, BITPOS)  (WORD|=1<<BITPOS)
#define SSETBIT(WORD, BITPOS) (WORD=1<<BITPOS)
#define CLRBIT(WORD, BITPOS)  (WORD&=~(1<<BITPOS))
#define SCLRBIT(WORD ,BITPOS) (WORD=1<<BITPOS)
#define CPLBIT(WORD, BITPOS)  (WORD^=1<<BITPOS)
#define READBIT(WORD, BITPOS) ((WORD>>BITPOS)&1)

#define WRITEBYTE(WORD,SBITPOS,BYTE)\
				(WORD=(WORD&~(255<<SBITPOS))|(BYTE<<SBITPOS))
				
#define WRITENIBBLE(WORD,SBITPOS,NIBBLE)\
				(WORD=(WORD&~(15<<SBITPOS))|(NIBBLE<<SBITPOS))
				
#define WRITEBIT(WORD,SBITPOS,BIT)\
				(WORD=(WORD&~(1<<SBITPOS))|(BIT<<SBITPOS))\
				
#define READBIT2(WORD, SBITPOS)     ((WORD >> BITPOS)&1)
#define READNIBBLE(WORD, SBITPOS)  ((WORD >> BITPOS)&15)
#define READBYTE(WORD, SBITPOS)    ((WORD >> BITPOS)&255)

#define READWRITEBIT(WORD,SBIT,DBIT)\
				((WORD=(WORD&~(1<<DBIT))|(((WORD>>SBITP)&1)<<DBIT))
				
#define READWRITEBIT2(DWORD,SWORD,SBIT,DBIT)\
				((DWORD=(DWORD&~(1<<DBIT))|(((SWORD>>SBITP)&1)<<DBIT))


//LCD pin defines

#define LCD_DATA 8 //pin0.8 to pin0.15
#define LCD_RS 16
#define LCD_RW 17
#define LCD_EN 18

//LCD Commands
#define CLEAR_LCD 0x01
#define RET_CUR_HOME 0X02
#define MODE_8BIT_1LINE 0X30
#define MODE_8BIT_2LINE 0X38
#define MODE_4BIT_1LINE 0X20
#define MODE_4BIT_2LINE 0X28
#define DISP_OFF 0X08
#define DISP_ON_CUR_OFF 0X0C
#define DISP_ON_CUR_ON 0X0E
#define DISP_ON_CUR_BLINK 0X0F
#define SHIFT_CUR_RIGHT 0X06
#define GOTO_LINE1_POS0 0X80
#define GOTO_LINE2_POS0 0XC0
#define GOTO_LINE3_POS0 0X94
#define GOTO_LINE4_POS0 0XD4
#define GOTO_CGRAM 0X40


//KPM defines.h

#define ROW0 16 //p1.16
#define ROW1 17 //p1.17
#define ROW2 18 //p1.18
#define ROW3 19 //p1.19

#define COL0 20 //p1.20
#define COL1 21 //p1.21
#define COL2 22 //p1.22
#define COL3 23 //p1.23


//ADC_defines.h

#define FOSC 12000000
#define CCLK (FOSC*5)
#define PCLK (CCLK/4)
#define ADCLK 3000000
#define CLKDIV_VALUE ((PCLK/ADCLK)-1)

//SFRs
//ADCR - A/D Control Register

#define CLKDIV 8
#define PDN_BIT 21
#define START_CONV 24

#define RESULT 6
#define DONE_BIT 31

#define CH0 1
#define CH1 2
#define CH2 4
#define CH3 8

//ADDR - A/D Data Register

#define AD0_1 0x01000000
#define AD0_2 0x04000000
#define AD0_3 0x10000000


//External Interrupt

#define Eint_LED 7
#define Eint_CH 14


//RTC 

//setting frequency

#define PREINT_VAL (((int)(PCLK/32768))-1)
#define PREFRAC_VAL (PCLK-((PREINT_VAL+1)*32768))

#define RTC_ENABLE (1<<0)
#define RTC_RESET (1<<1)
#define RTC_CLKSRC (1<<4)
