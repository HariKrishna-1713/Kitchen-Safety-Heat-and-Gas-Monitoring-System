#include "mini_project1_func.h"
#include "mini_project1_main.h"

//extern u32 flag;

int main()
{
	
	/*char* p=(char*)SECTOR_ADDR;
	char arr[10], arr1[10];
	WRITE_DATA_TO_FLASH();
	strcpy(arr,p);
	strcpy(arr1,p+5);*/
	
	Init_RTC();
	Init_LCD();
	Init_KPM();
	Init_ADC();
	Eint_Enable();
	init_timer0();
	
	IODIR0 |= GAS_PIN;
	IODIR0 |= 1<<LED;
	IODIR0 |= 1<<BUZZER;
	
	if(!Check_Password)
    {
        StrLCD("SET PASSWORD");
				WRITE_LCD_CMD(GOTO_LINE2_POS0);
				strcpy(StoredPassword, ReadPassword()); 
        WRITE_LCD_CMD(CLEAR_LCD);
    }
	while(1)
	{ 
		if(flag)
		{
			if(VerifyPassword())
			{
        WRITE_LCD_CMD(CLEAR_LCD);
			}
			Setting();
			flag=0;
			WRITE_LCD_CMD(CLEAR_LCD);
		}
		
		GetRTCTime(&hour, &min, &sec);
		DisplayRTCTime(hour,min,sec);
		
		GetRTCDate(&date, &mon, &year);
		WRITE_LCD_CMD(GOTO_LINE2_POS0);
		DisplayRTCDate(date,mon,year);
		
		GetRTCDay(&day);
		WRITE_LCD_CMD(GOTO_LINE1_POS0+9);
		DisplayRTCDay(day);
		
		DisplaySensorReading();
		DisplayEvent();
	}
}


