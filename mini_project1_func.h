#include <lpc214X.h>
#include "mini_project1.h"
#include "mini_project1_defines.h"
#include "string.h"
#include <stdlib.h>

#define MAX_PWD_LEN 10
#define EVENT_TEMP 1
#define EVENT_GAS  2
#define EVENT_BOTH 3

#define LED 7
#define BUZZER 5
#define SW  4
#define GAS_PIN 20

/* Runtime safety limit used by the temperature alarm decision logic. */
u32 TEMP_THRESHOLD=40;

/* Event snapshot state retained for the historical event display. */
u32 stamp_day  = 0;   
u32 stamp_temp = 0;   
u32 stamp_gas = 0;              
u32 EventActive = 0;  
u32 BuzzerAck = 1;  
u32 EventType = 0;
u32 hour,min,sec,day,mon,year,date;
u32 Event = 0;
u32 TempLevel=0, GasLevel=0;
u32 TempUnsafe=0, GasUnsafe=0;

/* Shared interrupt/event flag used to interrupt long display intervals. */
extern u32 flag;
s8 StoredPassword[MAX_PWD_LEN + 1];
s8 StoredNewPassword[MAX_PWD_LEN + 1];
u32 Check_Password=0;
s8 *NewPassword;
s8 *ConformPassword;
u32 position=0;
s8 rtc_set[3]={"00"};
s8 Timestamp[]={"00:00:00"};
s8 Datestamp[]={"00/00/00"};
s8 weekdays[][4]={"SUN","MON","TUE","WED","THU","FRI","SAT"};

/* RTC day-of-week indexes used by the weekday lookup table. */
#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6


/* Initialize the RTC peripheral and select the configured RTC clock source. */
void Init_RTC(void)
{
       CCR = RTC_RESET;
			 CCR = RTC_ENABLE | RTC_CLKSRC;
}


/* Collect a keypad password entry and present it as a masked LCD input. */
char *ReadPassword(void)
{
    static char pwd[MAX_PWD_LEN + 1];   // static: survives after the function returns
    u32 cnt = 0;
    u32 key;

    WRITE_LCD_CMD(GOTO_LINE2_POS0);
    WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

    while(1)
    {
        key = keyscan();
        delay_ms(150);

        if(key == '=' && cnt == 1)   // treat '=' as Enter/confirm
        {
            Check_Password = '1';

            break;
        }
        else if(key == 'c')   // treat 'c' as clear/cancel
        {
            cnt = 0;
            pwd[0] = '\0';
            WRITE_LCD_CMD(GOTO_LINE2_POS0);   // reset cursor, clear what was shown
            continue;
        }
        else if(key == '-')
				{
                                        if(cnt>0)
                                        {
                                                        pwd[cnt--] = (char)key;
                                                        WRITE_LCD_CMD(GOTO_LINE2_POS0+cnt);
                                                        WRITE_LCD_DATA(' ');
                                                        WRITE_LCD_CMD(GOTO_LINE2_POS0+cnt);
                                        }
                                }
        else if(cnt < MAX_PWD_LEN)   // only accept a digit if there's still room
        {
            pwd[cnt] = (char)key;
            cnt++;

            WRITE_LCD_DATA(key);
            delay_ms(100);
						WRITE_LCD_CMD(GOTO_LINE2_POS0 + cnt-1);
            WRITE_LCD_DATA('*');
						WRITE_LCD_CMD(GOTO_LINE2_POS0 + cnt);
						
						pwd[cnt] = '\0';
        }
    }

    WRITE_LCD_CMD(DISP_ON_CUR_OFF);
    WRITE_LCD_CMD(CLEAR_LCD);
    return pwd;
}

/* Authenticate the user and enforce the configured failed-attempt lockout policy. */
u32 VerifyPassword(void)
{
    u32 attempts = 0, countdown;
    char entered[MAX_PWD_LEN + 1];//+1 for null character

    while(1)
    {
        WRITE_LCD_CMD(CLEAR_LCD);
        WRITE_LCD_CMD(GOTO_LINE1_POS0);
        StrLCD("ENTER PASSWORD");

        strcpy(entered, ReadPassword());   // COPY out of the shared static buffer immediately

        if(strcmp(entered, StoredPassword) == 0)
        {
            return 1;   // correct -- caller can now proceed into Setting()
        }

        attempts++;

        WRITE_LCD_CMD(CLEAR_LCD);
        WRITE_LCD_CMD(GOTO_LINE1_POS0);
        StrLCD("ACCESS DENIED");
        if(attempts < 3)
            delay_s(1);

        if(attempts >= 3)
        {
            WRITE_LCD_CMD(GOTO_LINE2_POS0);
            StrLCD("SYS LOCKED ");

            for(countdown = 10; countdown > 0; countdown--)
            {
                WRITE_LCD_CMD(GOTO_LINE2_POS0 + 11);   // right after "SYS LOCKED "
                u32LCD(countdown);
                WRITE_LCD_DATA('s');
                WRITE_LCD_DATA(' ');   // clear leftover digit if countdown drops from 10 to 9
                tdelay_s(1);
                                                                while(T0MR0 != T0TC);
            }

            attempts = 0;   // reset -- give the user a fresh set of 3 chances after the lock
            WRITE_LCD_CMD(CLEAR_LCD);
        }
    }
}


/* Read the current RTC time into caller-provided variables. */
void GetRTCTime(u32 *hour, u32 *min, u32 *sec)
{
        *hour = HOUR;
        *min = MIN;
        *sec = SEC;
}

/* Render the supplied RTC time in HH:MM:SS format on the LCD. */
void DisplayRTCTime(u32 hour,u32 min,u32 sec)
{
        WRITE_LCD_CMD(GOTO_LINE1_POS0);
        WRITE_LCD_DATA(hour/10+48);
        WRITE_LCD_DATA(hour%10+48);
        WRITE_LCD_DATA(':');
        WRITE_LCD_DATA(min/10+48);
        WRITE_LCD_DATA(min%10+48);
        WRITE_LCD_DATA(':');
        WRITE_LCD_DATA(sec/10+48);
        WRITE_LCD_DATA(sec%10+48);
}

/* Read the current RTC calendar date into caller-provided variables. */
void GetRTCDate(u32 *date, u32 *mon, u32 *year)
{
        *date = DOM;
        *mon = MONTH;
        *year = YEAR;
}

/* Render the supplied RTC date in DD/MM/YY format on the LCD. */
void DisplayRTCDate(u32 date,u32 mon,u32 year)
{
        WRITE_LCD_CMD(GOTO_LINE2_POS0);
        WRITE_LCD_DATA(date/10+48);
        WRITE_LCD_DATA(date%10+48);
        WRITE_LCD_DATA('/');
        WRITE_LCD_DATA(mon/10+48);
        WRITE_LCD_DATA(mon%10+48);
        WRITE_LCD_DATA('/');
        u32LCD(year);
}

/* Read the RTC day-of-week index for display or event logging. */
void GetRTCDay(u32 *day)
{
        *day = DOW;
}

/* Convert the RTC day index to its configured three-character weekday label. */
void DisplayRTCDay(u32 day)
{
        WRITE_LCD_CMD(GOTO_LINE1_POS0+9);
        StrLCD(weekdays[day]);
}

/* Normalize the digital gas-sensor input to the application-level gas status. */
u32 GetGasStatus(void)
{
//Digital gas sensor: HIGH = gas present, LOW = gas absent.

		if((IOPIN0 >> GAS_PIN)&1)
        return 0;
    else
        return 1;
}

/* Update live sensor data, evaluate safety thresholds, and control alarm outputs. */
void DisplaySensorReading(void)
{
    WRITE_LCD_CMD(GOTO_LINE1_POS0 + 12);   
    TempLevel = LM35TempC();    // read live sensor value
    u32LCD(TempLevel);                         
    WRITE_LCD_DATA(0xDF);                  // degree symbol
    WRITE_LCD_DATA('C');

		WRITE_LCD_CMD(GOTO_LINE2_POS0 + 9);
		StrLCD("GAS:");
		GasLevel = GetGasStatus();
		WRITE_LCD_DATA(GasLevel + '0');

    TempUnsafe = (TempLevel > TEMP_THRESHOLD);
    GasUnsafe = (GasLevel == 1);

    if(TempUnsafe || GasUnsafe)
    {
        if(!EventActive)
        {
            EventActive = 1;
            Event = 1;
            BuzzerAck = 0;                 // a fresh event needs a fresh ack
                                                //delay_s(1);
            IOSET0 = 1<<LED;
            IOSET0 = 1<<BUZZER;

            // snapshot the moment into the stamp arrays
            GetRTCTime(&hour, &min, &sec);
            Timestamp[0]=hour/10+48; Timestamp[1]=hour%10+48;
            Timestamp[3]=min/10+48;  Timestamp[4]=min%10+48;
            Timestamp[6]=sec/10+48;  Timestamp[7]=sec%10+48;

            GetRTCDate(&date, &mon, &year);
            Datestamp[0]=date/10+48; Datestamp[1]=date%10+48;
            Datestamp[3]=mon/10+48;  Datestamp[4]=mon%10+48;
            Datestamp[6]=year/10+48; Datestamp[7]=year%10+48;

            GetRTCDay(&stamp_day);

            WRITE_LCD_CMD(CLEAR_LCD);
            StrLCD("UNSAFE");
            WRITE_LCD_CMD(GOTO_LINE2_POS0);
            if(TempUnsafe && GasUnsafe)
            {
                StrLCD("TEMP+GAS HIGH");
                stamp_temp = TempLevel;
                stamp_gas = GasLevel;
                EventType = EVENT_BOTH;
            }
            else if(TempUnsafe)
            {
                StrLCD("TEMP IS HIGH");
                stamp_temp = TempLevel;
                EventType = EVENT_TEMP;
            }
            else
            {
                StrLCD("GAS IS HIGH");
                stamp_gas = GasLevel;
                EventType = EVENT_GAS;
            }

             tdelay_s(2);
             while(T0MR0 != T0TC);
             WRITE_LCD_CMD(CLEAR_LCD);
        }
    }
    else
    {
        EventActive = 0;
        IOCLR0 = 1<<LED;
        IOCLR0 = 1<<BUZZER;   // safe to silence automatically once danger has passed
    }


    // switch check is non-blocking <97> one read per pass, not a spin loop
    if(!BuzzerAck && ((IOPIN0>>SW)&1))//high-switch
    {
        BuzzerAck = 1;
        IOCLR0 = 1<<BUZZER;
    }
}

/* Manage the live-status display and the most recently captured event display. */
void DisplayEvent(void)
{
    // ---- 10 seconds: normal continuous display ----
    tdelay_s(10);                       // arm a 10-second timer match
    while(T0MR0 != T0TC)                // loop until 10 seconds have elapsed
    {
				if(flag) return;
        GetRTCTime(&hour, &min, &sec);
        DisplayRTCTime(hour, min, sec);

        GetRTCDate(&date, &mon, &year);
        DisplayRTCDate(date, mon, year);

        GetRTCDay(&day);
        DisplayRTCDay(day);

        DisplaySensorReading();                 // live temp + event detection + buzzer ack, every pass
    }

    // ---- 3 seconds: show the LAST captured event, only once one has happened ----
    if(Event)
    {
        WRITE_LCD_CMD(CLEAR_LCD);

        WRITE_LCD_CMD(GOTO_LINE1_POS0);
        StrLCD(Timestamp);              // frozen time from the last event

        WRITE_LCD_CMD(GOTO_LINE1_POS0 + 9);
        DisplayRTCDay(stamp_day);       // frozen day (this function positions itself already)

        WRITE_LCD_CMD(GOTO_LINE2_POS0);
        StrLCD(Datestamp);              // frozen date from the last event

        if(EventType == EVENT_BOTH)
        {
                                        WRITE_LCD_CMD(GOTO_LINE1_POS0 + 12);
                                        u32LCD(stamp_temp);             // frozen temperature from the last event
                                        WRITE_LCD_DATA(0xDF);
                                        WRITE_LCD_DATA('C');

                                        WRITE_LCD_CMD(GOTO_LINE2_POS0 + 9);   // pick a free spot on your LCD layout
																				StrLCD("GAS:");
																				WRITE_LCD_DATA(stamp_gas + '0');
                                }
                                else if(EventType == EVENT_TEMP)
                                {
                                        WRITE_LCD_CMD(GOTO_LINE1_POS0 + 12);
                                        u32LCD(stamp_temp);             // frozen temperature from the last event
                                        WRITE_LCD_DATA(0xDF);
                                        WRITE_LCD_DATA('C');
                                }
                                else
                                {
                                        WRITE_LCD_CMD(GOTO_LINE2_POS0 + 9);   // pick a free spot on your LCD layout
																				StrLCD("GAS:");
																				WRITE_LCD_DATA(stamp_gas + '0');
                                }

        tdelay_s(3);                    // arm a 3-second timer match
        while(T0MR0 != T0TC)            // hold this screen for exactly 3 seconds
        {
                                         if(flag) return;
            // still allow the switch to silence the buzzer even during this frozen screen
            if(!BuzzerAck && ((IOPIN0>>SW)&1))
            {
                BuzzerAck = 1;
                IOCLR0 = 1<<BUZZER;
            }
        }

        WRITE_LCD_CMD(CLEAR_LCD);
    }
}

//RESET THRESHOLD VALUES

/* Capture a bounded numeric threshold from the keypad and return its value. */
u32 SetThreshold(u32 max_digits)
{
    u32 digits[4];
    u32 cnt = 0;
    u32 key;
    u32 value = 0;
    u32 i;

    while(1)
    {
        key = keyscan();
                delay_ms(150);
        if(key == '=' && cnt > 0)          // Enter/confirm
        {
            value = 0;

            for(i = 0; i < cnt; i++)
                value = (value * 10) + (digits[i] - 48);

                                          return value;
        }

        else if(key == 'c')                // Clear all digits
        {
            cnt = 0;
            value = 0;

            WRITE_LCD_CMD(GOTO_LINE2_POS0);
            continue;
        }

        else if(key == '-')                // Delete previous digit
        {
            if(cnt > 0)
            {
                cnt--;

                WRITE_LCD_CMD(GOTO_LINE2_POS0 + cnt);
                WRITE_LCD_DATA(' ');
                WRITE_LCD_CMD(GOTO_LINE2_POS0 + cnt);
            }
        }

        else if(key >= '0' && key <= '9')
        {
            if(cnt < max_digits)
            {
                digits[cnt] = key;
                cnt++;

                WRITE_LCD_DATA(key);
            }
        }
    }

}

/* Capture a two-digit RTC field from the keypad and return its numeric value. */
u32 RTC_SetValue(void)
{
    u32 key;
    u32 cnt = 0;

    rtc_set[0] = '0';
    rtc_set[1] = '0';
    rtc_set[2] = '\0';

    while(1)
    {
        key = keyscan();
        delay_ms(150);

        if(key == '-')
        {
            if(cnt > 0)
            {
                cnt--;

                WRITE_LCD_CMD(GOTO_LINE2_POS0 + cnt);
                WRITE_LCD_DATA(' ');
                WRITE_LCD_CMD(GOTO_LINE2_POS0 + cnt);
            }
        }

        else if(key == '=')
        {
						if(cnt == 1)
						{
							return ((0*10)+(rtc_set[0] - '0'));
						}
            else if(cnt == 2)
            {
                return ((rtc_set[0] - '0') * 10) +
                       (rtc_set[1] - '0');
            }
        }

        else if(key >= '0' && key <= '9')
        {
            if(cnt < 2)
            {
                rtc_set[cnt] = key;

                WRITE_LCD_DATA(key);

                cnt++;
            }
        }
    }
}

/* Provide the protected configuration menu for RTC, thresholds, password, and exit. */
void Setting(void)
{
    u32 num, num1;
    u32 daydata;
    u32 value;

edit:

    WRITE_LCD_CMD(CLEAR_LCD);

    WRITE_LCD_CMD(GOTO_LINE1_POS0);
    StrLCD("1.SetRTC");

    WRITE_LCD_CMD(GOTO_LINE2_POS0);
    StrLCD("2.SetP");

    WRITE_LCD_CMD(GOTO_LINE1_POS0 + 9);
    StrLCD("3.RsetP");

    WRITE_LCD_CMD(GOTO_LINE2_POS0 + 9);
    StrLCD("4.EXIT");

    num = keyscan();
    delay_ms(150);


    /* =====================================================
                         MAIN MENU
       ===================================================== */

    if(num == '1' || num == '2' || num == '3' || num == '4')
    {
        switch(num)
        {

            /* =================================================
                              SET RTC
               ================================================= */

            case '1':

time_menu:

                WRITE_LCD_CMD(CLEAR_LCD);

                WRITE_LCD_CMD(GOTO_LINE1_POS0);
                StrLCD("1.SETTIME");

                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                StrLCD("2.SetDate");

                WRITE_LCD_CMD(GOTO_LINE1_POS0 + 10);
                StrLCD("3.SetD");

                WRITE_LCD_CMD(GOTO_LINE2_POS0 + 10);
                StrLCD("4.Back");

                num1 = keyscan();
                delay_ms(150);


                if(num1 == '1')
                {

                    /* =========================================
                                   TIME MENU
                       ========================================= */

hour_min_sec:

                    WRITE_LCD_CMD(CLEAR_LCD);

                    WRITE_LCD_CMD(GOTO_LINE1_POS0);
                    StrLCD("1.HOUR");

                    WRITE_LCD_CMD(GOTO_LINE2_POS0);
                    StrLCD("2.MIN");

                    WRITE_LCD_CMD(GOTO_LINE1_POS0 + 8);
                    StrLCD("3.SEC");

                    WRITE_LCD_CMD(GOTO_LINE2_POS0 + 8);
                    StrLCD("4.BACK");

                    num1 = keyscan();
                    delay_ms(150);


                    switch(num1)
                    {

                        /* =====================================
                                      HOUR
                           ===================================== */

											hour:case '1':

                            WRITE_LCD_CMD(CLEAR_LCD);

                            StrLCD("SET HOUR");

                            WRITE_LCD_CMD(GOTO_LINE2_POS0);

                            WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                            value = RTC_SetValue();

                            WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                            if(value <= 23)
                            {
                                HOUR = value;

                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("HOUR SET");
                                delay_s(1);
                            }
                            else
                            {
                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("INVALID HOUR");

                                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                                StrLCD("RANGE 00-23");

                                delay_s(1);
															  goto hour;
                            }

                            goto hour_min_sec;


                        /* =====================================
                                      MINUTE
                           ===================================== */

											min:case '2':

                            WRITE_LCD_CMD(CLEAR_LCD);

                            StrLCD("SET MIN");

                            WRITE_LCD_CMD(GOTO_LINE2_POS0);

                            WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                            value = RTC_SetValue();

                            WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                            if(value <= 59)
                            {
                                MIN = value;

                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("MIN SET");
                                delay_s(1);
                            }
                            else
                            {
                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("INVALID MIN");

                                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                                StrLCD("RANGE 00-59");

                                delay_s(1);
																goto min;
                            }

                            goto hour_min_sec;


                        /* =====================================
                                      SECOND
                           ===================================== */

											sec:case '3':

                            WRITE_LCD_CMD(CLEAR_LCD);

                            StrLCD("SET SEC");

                            WRITE_LCD_CMD(GOTO_LINE2_POS0);

                            WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                            value = RTC_SetValue();

                            WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                            if(value <= 59)
                            {
                                SEC = value;

                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("SEC SET");
                                delay_s(1);
                            }
                            else
                            {
                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("INVALID SEC");

                                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                                StrLCD("RANGE 00-59");

                                delay_s(1);
															  goto sec;
                            }

                            goto hour_min_sec;


                        case '4':
														WRITE_LCD_CMD(CLEAR_LCD);
                            goto time_menu;
                    }

                    goto hour_min_sec;
                }


                /* =============================================
                                  DATE
                   ============================================= */

                else if(num1 == '2')
                {

date_menu:

                    WRITE_LCD_CMD(CLEAR_LCD);

                    WRITE_LCD_CMD(GOTO_LINE1_POS0);
                    StrLCD("1.DOM");

                    WRITE_LCD_CMD(GOTO_LINE2_POS0);
                    StrLCD("2.MON");

                    WRITE_LCD_CMD(GOTO_LINE1_POS0 + 8);
                    StrLCD("3.YEAR");

                    WRITE_LCD_CMD(GOTO_LINE2_POS0 + 8);
                    StrLCD("4.BACK");

                    num1 = keyscan();
                    delay_ms(150);


                    switch(num1)
                    {

                        /* =====================================
                                      DATE / DOM
                           ===================================== */

											Dom:case '1':

                            WRITE_LCD_CMD(CLEAR_LCD);

                            StrLCD("SET DOM");

                            WRITE_LCD_CMD(GOTO_LINE2_POS0);

                            WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                            value = RTC_SetValue();

                            WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                            if(value >= 1 && value <= 31)
                            {
                                DOM = value;

                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("DOM SET");
                                delay_s(1);
                            }
                            else
                            {
                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("INVALID DOM");

                                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                                StrLCD("RANGE 01-31");

                                delay_s(1);
															  goto Dom;
                            }

                            goto date_menu;


                        /* =====================================
                                      MONTH
                           ===================================== */

											mon:case '2':

                            WRITE_LCD_CMD(CLEAR_LCD);

                            StrLCD("SET MONTH");

                            WRITE_LCD_CMD(GOTO_LINE2_POS0);

                            WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                            value = RTC_SetValue();

                            WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                            if(value >= 1 && value <= 12)
                            {
                                MONTH = value;

                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("MONTH SET");
                                delay_s(1);
                            }
                            else
                            {
                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("INVALID MONTH");

                                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                                StrLCD("RANGE 01-12");

                                delay_s(1);
															  goto mon;
                            }

                            goto date_menu;


                        /* =====================================
                                      YEAR
                           ===================================== */

										 year:case '3':

                            WRITE_LCD_CMD(CLEAR_LCD);

                            StrLCD("SET YEAR");

                            WRITE_LCD_CMD(GOTO_LINE2_POS0);

                            WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                            value = RTC_SetValue();

                            WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                            if(value <= 99)
                            {
                                YEAR = value;

                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("YEAR SET");
                                delay_s(1);
                            }
                            else
                            {
                                WRITE_LCD_CMD(CLEAR_LCD);
                                StrLCD("INVALID YEAR");

                                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                                StrLCD("RANGE 00-99");

                                delay_s(1);
																goto year;
                            }

                            goto date_menu;


                        case '4':
														WRITE_LCD_CMD(CLEAR_LCD);
                            goto time_menu;
                    }

                    goto date_menu;
                }


                /* =============================================
                                  SET DAY
                   ============================================= */

                else if(num1 == '3')
                {

                    WRITE_LCD_CMD(CLEAR_LCD);

                    StrLCD("SET DAY");

                    WRITE_LCD_CMD(GOTO_LINE2_POS0);

                    WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                    while(1)
                    {
                        daydata = keyscan();
                        delay_ms(150);

                        if(daydata >= '0' && daydata <= '6')
                        {
                            DOW = daydata - '0';

                            WRITE_LCD_DATA(daydata);

                            break;
                        }

                        WRITE_LCD_CMD(CLEAR_LCD);

                        StrLCD("VALID DAY");

                        WRITE_LCD_CMD(GOTO_LINE2_POS0);
                        StrLCD("RANGE 0-6");

                        delay_s(1);

                        WRITE_LCD_CMD(CLEAR_LCD);

                        StrLCD("SET DAY");

                        WRITE_LCD_CMD(GOTO_LINE2_POS0);
                    }

                    WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                    goto time_menu;
                }


                else if(num1 == '4')
                {
										WRITE_LCD_CMD(CLEAR_LCD);
                    goto edit;
                }

                goto time_menu;


            /* =================================================
                              SET PARAMETERS
               ================================================= */

         case '2':

								WRITE_LCD_CMD(CLEAR_LCD);
								WRITE_LCD_CMD(GOTO_LINE1_POS0);
								StrLCD("SET TEMP LIMIT");
								WRITE_LCD_CMD(GOTO_LINE2_POS0);
                WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                TEMP_THRESHOLD = SetThreshold(2);

                WRITE_LCD_CMD(DISP_ON_CUR_OFF);
                WRITE_LCD_CMD(CLEAR_LCD);

                StrLCD("TEMP LIMIT SET");

                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                u32LCD(TEMP_THRESHOLD);

                delay_s(1);

                goto edit;
								
            /* =================================================
                              RESET PASSWORD
               ================================================= */

            case '3':

                VerifyPassword();

Password:

                WRITE_LCD_CMD(GOTO_LINE1_POS0);
                StrLCD("RESET PASSWORD");

                WRITE_LCD_CMD(GOTO_LINE2_POS0);
                WRITE_LCD_CMD(DISP_ON_CUR_BLINK);

                NewPassword = ReadPassword();

                if(strcmp(StoredPassword,NewPassword) == 0)
                {
                    WRITE_LCD_CMD(CLEAR_LCD);
                    StrLCD("Enter New Pass");

                    delay_s(1);

                    WRITE_LCD_CMD(CLEAR_LCD);

                    goto Password;
                }

                strcpy(StoredNewPassword,NewPassword);
								
ConformPassword:WRITE_LCD_CMD(CLEAR_LCD);
                StrLCD("CONFORM PASSWORD");
								WRITE_LCD_CMD(GOTO_LINE2_POS0);
								WRITE_LCD_CMD(DISP_ON_CUR_BLINK);
								
								ConformPassword = ReadPassword();
								if(strcmp(StoredNewPassword,ConformPassword) == 0)
                {
                    strcpy(StoredPassword,NewPassword);
										WRITE_LCD_CMD(CLEAR_LCD);
										StrLCD("PASSWORD RESET");

										delay_ms(500);
                }
								else
								{
										WRITE_LCD_CMD(CLEAR_LCD);
                    StrLCD("PASSWORD");
										WRITE_LCD_CMD(GOTO_LINE2_POS0);
										StrLCD("NOT MATCH");
                    delay_s(1);

                    WRITE_LCD_CMD(CLEAR_LCD);

                    goto ConformPassword;
								}

                WRITE_LCD_CMD(DISP_ON_CUR_OFF);

                WRITE_LCD_CMD(CLEAR_LCD);

                goto edit;


            /* =================================================
                                  EXIT
               ================================================= */

            case '4':
								WRITE_LCD_CMD(CLEAR_LCD);
                return;
        }
    }

    WRITE_LCD_CMD(CLEAR_LCD);
}
