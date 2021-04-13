/**********************************************************
Semester      : Spring-2021
Course   		  : EE340 (EDL)
Project Title : SoC Measurement

Group 15

Jay Sawant         : 18D070050
Prashant Shettigar : 18D070063
Chinmay Bharti     : 18D070043

***********************************************************/
#pragma large
#include <at89c5131.h>
#include "lcd.h"																				//Driver for interfacing lcd 
#include "mcp3008.h"																		//Driver for interfacing ADC ic MCP3008

sbit L1 = P3^1;
char adc_ascii[6]={0,0,0,0,'\0'};							
code unsigned char display_msg1[]="1:       2:";						
code unsigned char display_msg2[]="3:       4:";
unsigned char display_msg3[]={0,0,'.',0,0,'\0'};
unsigned int count = 0;					
float soc[4]={99.99,99.99,99.99,99.99};	

void Timer(void) interrupt 1 			// Interrupt No.1 for Timer 0
{
	count = count + 1; 				
}

void soc_update(int id,float v,float Vh,float Vl)  
{
		if (v>Vh)																			// If voltage of the cell > upper threshold, it is considered as fully charged
		{
			soc[id] = 99.99;
		}
		if (v<Vl)																			// If voltage of the cell < lower threshold, it is considered as empty
		{
			soc[id] = 0;
		}
}
void up_disp(void)																// Helper function for displaying SoC on LCD
{
		display_msg3[0] = adc_ascii[0];							
		display_msg3[1] = adc_ascii[1];
		display_msg3[3] = adc_ascii[2];
		display_msg3[4] = adc_ascii[3];		 
}
void main(void)
{
	float Vh;
	float Vl;
	int i;
	float Q_r = 43200.0;						//12 A-hr ====> 43200 C (capacity)
	Vh = 12.55;											//Upper threshold of voltage of cell
	Vl = 9.9;												//Lower threshold of voltage of cell

	L1 = 0;													//Output LED = OFF
	
	spi_init();											// Serial Peripheral Interface Initialisation 													
	adc_init();											// ADC Initialisation
  lcd_init();											// LCD initialisation
	
	lcd_cmd(0x80);																		
	lcd_write_string(display_msg1);	
	lcd_cmd(0xC0);																		
	lcd_write_string(display_msg2);	
	
	TMOD = 0x01;												// Timer0 ===> Usd for calculating delta_t
	TH0= 0x00;  			
	TL0 = 0x00;
	ET0 = 1;
	while(1)
	{
		float d_Q = 0;
		float v1,v2,v3,v4;
		float d_t;
		float curr;		
		
		curr = adc(0)*9.7752*0.001;     	// current in A (3.3 ---> 10A) current(in Amperes) = adc(0)*(3.3/1023)*(10/3.3)
		msdelay(1);
		v1 = adc(1)*12.3167*0.001;				// volt in V (3.3--->12.6V) voltage(in Volts) = adc(0)*(3.3/1023)*(12.6/3.3)
		msdelay(1);
		v2 = adc(2)*12.3167*0.001;
		msdelay(1);
		v3 = adc(3)*12.3167*0.001;
		msdelay(1);
		v4 = adc(4)*12.3167*0.001;
		msdelay(1);    								
		
		TR0 = 0;                                                       //stopping the timer to calculate delta_t
		d_t = count*32.7675 + ((TL0) + 256*(TH0))*0.0005;      				 //delta_t in milli sec
		d_Q =  curr*d_t/1000.0;                                        //delta_Q in Coulombs
		TH0 = 0x00;                                                    //Initialising the timer again to measure delta_t in next iteration
		TL0 = 0x00;
		count = 0;
		TR0 = 1;
			
		for(i=0;i<4;i++)
		{
			soc[i] = soc[i] - (d_Q/Q_r)*100.0;	               // Q_r (Rated capacity) is in Coulombs		
		}																										 // SoC of all 4 batteries is updated almost every 25 ms
		
		soc_update(0,v1,Vh,Vl);															 // Checking that the voltage of each cell is within the range or not  
		soc_update(1,v2,Vh,Vl);															 // and updating the SoC accordingly.
		soc_update(2,v3,Vh,Vl);
		soc_update(3,v4,Vh,Vl);

		int_to_string((unsigned int)(soc[0]*100.0),adc_ascii);	//displaying the calculated SoC of all 4 batteries on LCD	
		up_disp();
		lcd_cmd(0x82);		
		lcd_write_string(display_msg3);			
		int_to_string((unsigned int)(soc[1]*100.0),adc_ascii);		
		up_disp();
		lcd_cmd(0x8B);		
		lcd_write_string(display_msg3);	
		int_to_string((unsigned int)(soc[2]*100.0),adc_ascii);		
		up_disp();
		lcd_cmd(0xC2);		
		lcd_write_string(display_msg3);	
		int_to_string((unsigned int)(soc[3]*100.0),adc_ascii);		
		up_disp();
		lcd_cmd(0xCB);		
		lcd_write_string(display_msg3);

		if (soc[0]<10|soc[1]<10|soc[2]<10|soc[3]<10)						// Toggling of LED if any of the calculated SoC's fall below 10%
		{
			L1 = ~L1;
		}

		msdelay(20);																						// 20 milli seconds delay
	}
}