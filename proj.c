#include<lpc17xx.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

void lcd_init(void);
void write(int, int);
void delay_lcd(unsigned int);
void lcd_comdata(int, int); 
void clear_ports(void);
void rtc_init(void);
void lcd_puts(unsigned char *);
void disp_time(void);
void sound_Buzzer(void);
void scan(void);
void keyboard(void);
void flash_time(void);
unsigned int l=0,int_key=0;
unsigned char row, var, flag, key;
unsigned long int i, var1, temp, temp1, temp2, temp3;
unsigned char SCAN_CODE[10] = {0x11, 0x21, 0x41, 0x81, 0x12, 0x22, 0x42, 0x82, 0x14, 0x24};
unsigned char ASCII_CODE[10] = {'0', '1', '2', '3','4', '5', '6', '7', '8', '9'};
unsigned char key2[2]={'0','\0'};
char alarmtime[15];
char timestring[15];
char datestring[15];
unsigned char un_alarmtime[15];
unsigned char un_timestring[15];
unsigned char un_datestring[15];
#define SBIT_CLKEN     0    /* RTC Clock Enable*/
#define SBIT_CTCRST    1    /* RTC Clock Reset */
#define SBIT_CCALEN    4    /* RTC Calibration counter enable */


unsigned char wel_msg1[16] 		= {"WELCOME TO"};
unsigned char wel_msg2[16] 		= {"LPC ALARM CLOCK"};
unsigned char alarm_msg1[16]	=	{" SET ALARM TIME"};
unsigned char alarm_msg2[16] 	= {"ALARM SET FOR"};
unsigned char error_msg1[16] 	= {"INVALID TIME"};
unsigned char error_msg2[16] 	=	{"SET VALID TIME"};
unsigned int hour, min, sec, date, month, year;
unsigned int alarm_hour=0, alarm_min=0, alarm_sec=0,count=0,buz_flag=0,buz_cond=0;

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate(); 
	lcd_init();
	
	lcd_comdata(0x80, 0);
	delay_lcd(800);
	lcd_puts(&wel_msg1[0]);
	
	lcd_comdata(0xC0, 0);
	delay_lcd(800);
	lcd_puts(&wel_msg2[0]);
	delay_lcd(1000000);
	
	lcd_comdata(0x0C, 0);
	rtc_init();// Initialize RTC module
	flash_time();
	lcd_comdata(0x80, 0);
	delay_lcd(800);
	lcd_comdata(0x01, 0);
	lcd_puts(&alarm_msg1[0]);
	
	// Initialize buzzer
	LPC_PINCON->PINSEL0 &= ~(3<<22);
	LPC_GPIO0->FIODIR|=0x800;
	// Initialize GPIO pins for input and output for keyboard
  LPC_GPIO2->FIODIR |= 0x00003C00; // Output: P2.10 to P2.13 (rows)
  LPC_GPIO1->FIODIR &= 0xF87FFFFF; // Input: P1.23 to P1.26 (cols)
  LPC_GPIO0->FIODIR |= 0x0F << 23 | 1 << 27 | 1 << 28;
	
	count = 0; 
	lcd_comdata(0xC0,0);	
    while (count<6)
    {
        while (1)
        {
            for (row = 1; row < 5; row++)
            {
                if (row == 1)
                    var1 = 0x00000400;
                else if (row == 2)
                    var1 = 0x00000800;
                else if (row == 3)
                    var1 = 0x00001000;
                else if (row == 4)
                    var1 = 0x00002000;
                temp = var1;
                LPC_GPIO2->FIOCLR = 0x00003C00;
                LPC_GPIO2->FIOSET = var1;
                flag = 0;
                scan();
                if (flag == 1)
                {
                    count++;
                    break;
                }
            } 
            if (flag == 1)
                break;
        } 
        for (i = 0; i < 10; i++) 
        {
            if (key == SCAN_CODE[i])
            {
                key = ASCII_CODE[i];
								key2[0]=ASCII_CODE[i];
								key2[1]='\0';
								lcd_puts(key2);
								delay_lcd(100000);
								int_key	=	key-48;
                if(count==1)
								{
                    alarm_hour = alarm_hour + int_key*10;
                }
                else if (count==2)
								{
                    alarm_hour = alarm_hour + int_key;
                }
                else if( count==3)
								{
                    alarm_min = alarm_min + int_key*10;
                }
                else if( count == 4)
								{
                    alarm_min = alarm_min + int_key;
                }
								else if( count==5)
								{
                    alarm_sec = alarm_sec + int_key*10;
                }
                else if( count == 6)
								{
                    alarm_sec = alarm_sec + int_key;
                }
                break;
            } 
        } 
		} 	//calling keyboard function to read input
		if (alarm_hour > 23 || alarm_min > 59 || alarm_sec > 59) // Check if hour or minute values are out of range
    {
      lcd_comdata(0x80,0); // Move to the first line of LCD
      delay_lcd(800);
			lcd_puts(&error_msg1[0]); // Display error message
      lcd_comdata(0xC0,0); // Move to the first line of LCD
      delay_lcd(800);
			lcd_puts(&error_msg2[0]); // Display error message
    }
	
	LPC_RTC->ALHOUR = alarm_hour;
  LPC_RTC->ALMIN 	= alarm_min;
  LPC_RTC->ALSEC 	= alarm_sec;
    
  // Enable alarm interrupt
  LPC_RTC->AMR = 0x00; // Alarm match triggers interrupt
  NVIC_EnableIRQ(RTC_IRQn); // Enable RTC interrupt in NVIC
	
	sprintf(alarmtime,"%02d:%02d:%02d", alarm_hour, alarm_min, alarm_sec);
	for (l = 0;l<sizeof(alarmtime); l++) 
	{
    un_alarmtime[l] = (unsigned char)alarmtime[l];
	}
	lcd_comdata(0x01, 0);
	delay_lcd(800);
	lcd_comdata(0x80, 0);
	delay_lcd(800);
	lcd_puts(&alarm_msg2[0]);
	lcd_comdata(0xC0, 0);
	delay_lcd(800);
	lcd_puts(&un_alarmtime[0]);
	delay_lcd(3000000);
	disp_time();
}
void flash_time(void)
{
			int m;
			for(m=0;m<100;m++)
			{
			hour = LPC_RTC->HOUR;
      min  = LPC_RTC->MIN; 
			sec  = LPC_RTC->SEC; 

      /* Read Date */
      date  = LPC_RTC->DOM;   
      month = LPC_RTC->MONTH;  
      year  = LPC_RTC->YEAR;   

      /* Display date and time on LCD */
      lcd_comdata(0x80, 0);  		/* Go to First line of 2x16 LCD */
		  delay_lcd(3200);
			sprintf(timestring, "Time: %02d:%02d:%02d", hour, min, sec);
			for (l = 0;l<sizeof(timestring); l++) 
			{
        un_timestring[l] = (unsigned char)timestring[l];
			}
			lcd_puts(&un_timestring[0]);
      lcd_comdata(0xC0, 0);            /* Go to Second line of 2x16 LCD */
		  delay_lcd(3200);
			sprintf(datestring, "Date:%02d/%02d/%04d", date, month, year);
			for (l = 0;l<sizeof(datestring); l++) 
			{
        un_datestring[l] = (unsigned char)datestring[l];
			}			
			lcd_puts(&un_datestring[0]);
		}
}
//display date and time
void disp_time(void)
{
	while (1)
  {
			/* Read Time */
      hour = LPC_RTC->HOUR;
      min  = LPC_RTC->MIN; 
			sec  = LPC_RTC->SEC; 

      /* Read Date */
      date  = LPC_RTC->DOM;   
      month = LPC_RTC->MONTH;  
      year  = LPC_RTC->YEAR;   

      /* Display date and time on LCD */
      lcd_comdata(0x80, 0);  		/* Go to First line of 2x16 LCD */
		  delay_lcd(3200);
			sprintf(timestring, "Time: %02d:%02d:%02d", hour, min, sec);
			for (l = 0;l<sizeof(timestring); l++) 
			{
        un_timestring[l] = (unsigned char)timestring[l];
			}
			lcd_puts(&un_timestring[0]);
      lcd_comdata(0xC0, 0);            /* Go to Second line of 2x16 LCD */
		  delay_lcd(3200);
			sprintf(datestring, "Date:%02d/%02d/%04d", date, month, year);
			for (l = 0;l<sizeof(datestring); l++) 
			{
        un_datestring[l] = (unsigned char)datestring[l];
			}			
			lcd_puts(&un_datestring[0]);
		
			if (alarm_hour==LPC_RTC->HOUR && alarm_min ==  LPC_RTC->MIN && alarm_sec==LPC_RTC->SEC) 
			{
				buz_flag=1;
				// Clear alarm interrupt flag
				LPC_RTC->ILR |= (1 << 1);
			}
			sound_Buzzer();
	}   
}
//rtc initialization
void rtc_init()
{
	 LPC_RTC->CCR = ((1 << SBIT_CTCRST ) | (1 << SBIT_CCALEN));
   LPC_RTC->CALIBRATION = 0x00;
   LPC_RTC->CCR = (1 << SBIT_CLKEN);	// Enable the clock for RTC 
	 
	 // Set Date and Time only once, comment these lines after setting the time and date                                         
   // Set Date 14th Nov 2015 
   LPC_RTC->DOM    = 03;   // Update date value 
   LPC_RTC->MONTH  = 04;   // Update month value
   LPC_RTC->YEAR   = 2024; // Update year value

   // Set Time 10:40:25 AM 
   //LPC_RTC->HOUR   = 16;   // Update hour value 
   //LPC_RTC->MIN    = 23;   // Update min value
	 //LPC_RTC->SEC    = 00;   // Update sec value 
}
//sound the buzzer
void sound_Buzzer()
{
	if(buz_flag==1)
	{
		if(buz_cond%2==0)
		{
			LPC_GPIO0->FIOSET=0x800; //buzzer on 
			buz_cond++;
		}
		else if(buz_cond%2!=0)
		{
			LPC_GPIO0->FIOCLR=0x800; //buzzer off
			buz_cond++;
		}
		if(buz_cond==175)
			buz_flag=0;
	}
}
//lcd initialization
void lcd_init()
{
	/*Ports initialized as GPIO */
	LPC_PINCON->PINSEL1 &= 0xFC003FFF; //P0.23 to P0.28
	/*Setting the directions as output */
	LPC_GPIO0->FIODIR |= 0x0F<<23 | 1<<27 | 1<<28;
	clear_ports();
	delay_lcd(3200);
	lcd_comdata(0x33, 0); 
	delay_lcd(30000); 
	lcd_comdata(0x32, 0);
	delay_lcd(30000);
	lcd_comdata(0x28, 0); //function set
	delay_lcd(30000);
	lcd_comdata(0x0c, 0);//display on cursor off
	delay_lcd(800);
	lcd_comdata(0x06, 0); //entry mode set increment cursor right
	delay_lcd(800);
	lcd_comdata(0x01, 0); //display clear
	delay_lcd(10000);
	return;
}
void lcd_comdata(int temp1, int type)
{
	int temp2 = temp1 & 0xf0; //move data (26-8+1) times : 26 - HN place, 4 - Bits
	temp2 = temp2 << 19; //data lines from 23 to 26
	write(temp2, type);
	temp2 = temp1 & 0x0f; //26-4+1
	temp2 = temp2 << 23; 
	write(temp2, type);
	delay_lcd(1000);
	return;
}
void write(int temp2, int type) //write to command/data reg
{ 
	clear_ports();
	LPC_GPIO0->FIOPIN = temp2; // Assign the value to the data lines 
	if(type==0)
		LPC_GPIO0->FIOCLR = 1<<27; // clear bit RS for Command
	else
		LPC_GPIO0->FIOSET = 1<<27; // set bit RS for Data
	LPC_GPIO0->FIOSET = 1<<28; // EN=1
	delay_lcd(25);
	LPC_GPIO0->FIOCLR = 1<<28; // EN =0
	return;
}
void delay_lcd(unsigned int r1)
{
	unsigned int r;
	for(r=0;r<r1;r++);
	return;
}
void clear_ports(void)
{
	/* Clearing the lines at power on */
	LPC_GPIO0->FIOCLR = 0x0F<<23; //Clearing data lines
	LPC_GPIO0->FIOCLR = 1<<27; //Clearing RS line
	LPC_GPIO0->FIOCLR = 1<<28; //Clearing Enable line
	return;
}
void lcd_puts(unsigned char *buf1)
{
	unsigned int i=0;
	unsigned int temp3;
	while(buf1[i]!='\0')
	{
		temp3 = buf1[i];
		lcd_comdata(temp3, 1);
		i++;
		if(i==16)
		{
			lcd_comdata(0xc0, 0);
		}
 }
 return;
}


void scan(void)
{
    temp3 = LPC_GPIO1->FIOPIN;
    temp3 &= 0x07800000; 
    if (temp3 != 0x00000000)
    {
        flag = 1;
        temp3 >>= 19;       
        temp >>= 10;        
        key = temp3 | temp;
    } 
}
