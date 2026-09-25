//types

typedef unsigned int u32;
typedef unsigned char u8;
typedef int s32;
typedef char s8;
typedef unsigned short int u16;
typedef short int s16;
typedef float f32;
typedef double f64;


//hardware (timer)dealy:

void init_timer0(void);
void tdelay_us(u32 us);
void tdelay_ms(u32 ms);
void tdelay_s(u32 s);

//Delay

void delay_us(u32 dlyus);
void delay_ms(u32 dlyms);
void delay_s(u32 dlys);


//LCD

void WRITE_LCD_CMD(u8 cmd);
void Init_LCD(void);
void WRITE_LCD_DATA(u8 ascii);
void StrLCD(s8* str);
void BuildcgRAM1(u8*, u8);
void u32LCD(u32);


//External_Interrupt

void Eint_isr(void) __irq;
void Eint_Enable(void);

//Keypad

void Init_KPM(void);
u32 colscan(void);
u32 rowcheck(void);
u32 colcheck(void);
u32 keyscan(void);

//ADC

void Init_ADC(void);
void Read_ADC(u32 chno, u32* dval, f32* eAR);
u32 LM35TempC(void);
u32 MQ2GasLevel_ppm(void);

