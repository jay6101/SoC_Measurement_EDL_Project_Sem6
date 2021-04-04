/**********************************************************
EE340: EDL Project (SoC Measurement)

***********************************************************/
#pragma large
#include <at89c5131.h>
#include "lcd.h"																				//Driver for interfacing lcd 
#include "mcp3008.h"																		//Driver for interfacing ADC ic MCP3008
char adc_ip_data_ascii[6]={0,0,0,0,'\0'};							
code unsigned char display_msg1[]="SoC: ";						
code unsigned char display_msg2[]=" %";	
code unsigned char display_msg4[]="batt empty";
code unsigned char display_msg5[]="batt full";
unsigned char display_msg3[]={0,0,'.',0,0,'\0'};
unsigned int count = 0;					

void Timer(void) interrupt 1 			// Interrupt No.1 for Timer 0
{
	count = count + 1; 				
}

void main(void)
{
	float Vh;
	float Vl;
	float SOC;
	float Q_r;
	Vh = 12.6;
	Vl = 9.9;
	SOC = 100.0;
	Q_r = 43200.0;     //12 A-hr
	spi_init();																					
	adc_init();
  lcd_init();
	lcd_cmd(0x80);																		
	lcd_write_string(display_msg1);	
	TMOD = 0x01;				// Mode1 of Timer0
	TH0= 0x00;  			
	TL0 = 0x00;
	ET0 = 1;
	while(1)
	{
		float d_Q = 0;
		float volt;
		float d_t;
		float curr;																			
		curr = adc(0)*9.7752*0.001;     // current in A   (3.3 ---> 10A)(3.3--->12.6V)
		volt = adc(7)*12.3167*0.001;				// volt in volts//scale further to get the analog values
		msdelay(250);    //delay 250ms
		if (volt<Vl){
					lcd_cmd(0xC0);		
					lcd_write_string(display_msg4);	
					SOC = 0;
		}
		else{
					if (volt>=Vh){
							lcd_cmd(0xC0);		
							lcd_write_string(display_msg5);	
							SOC = 100.0;	
					}
					else{
							TR0 = 0;                                                       //delta_t// calculate delta t using TH0 TL0
							d_t = count*32.7675 + ((TL0<<8) + 256*(TH0<<8))*0.0005;      //delta_t in ms
							d_Q =  curr*d_t;                                               // delta_Q in mC
							TH0 = 0x00;                                                    //Initialising the timer again to measure delta_t
							TL0 = 0x00;
							count = 0;
							TR0 = 1;
					}
		}	
		SOC = SOC - (d_Q/Q_r)*100;	      // Q rated in mC	, SoC updated almost every 250 ms																									
		int_to_string((unsigned int) (SOC*100.0),adc_ip_data_ascii);									
		display_msg3[0] = adc_ip_data_ascii[0];							
		display_msg3[1] = adc_ip_data_ascii[1];
		display_msg3[3] = adc_ip_data_ascii[2];
		display_msg3[4] = adc_ip_data_ascii[3];		 
		lcd_cmd(0x86);		
		lcd_write_string(display_msg3);	
		msdelay(10);
	}
}