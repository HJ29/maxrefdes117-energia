/** \file RD117_LILYPAD.ino ******************************************************
*
* Project: MAXREFDES117#
* Filename: RD117_LILYPAD.ino
* Description: This module contains the Main application for the MAXREFDES117 example program.
*
* Revision History:
*\n 1-18-2016 Rev 01.00 GL Initial release.
*\n
*
* --------------------------------------------------------------------
*
* This code follows the following naming conventions:
*
* char              ch_pmod_value
* char (array)      s_pmod_s_string[16]
* float             f_pmod_value
* int32_t           n_pmod_value
* int32_t (array)   an_pmod_value[16]
* int16_t           w_pmod_value
* int16_t (array)   aw_pmod_value[16]
* uint16_t          uw_pmod_value
* uint16_t (array)  auw_pmod_value[16]
* uint8_t           uch_pmod_value
* uint8_t (array)   auch_pmod_buffer[16]
* uint32_t          un_pmod_value
* int32_t *         pn_pmod_value
*
* ------------------------------------------------------------------------- */
/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
/*!\mainpage Main Page
*
* \section intro_sec Introduction
*
* This is the code documentation for the MAXREFDES117# subsystem reference design.
* 
*  The Files page contains the File List page and the Globals page.
* 
*  The Globals page contains the Functions, Variables, and Macros sub-pages.
*
* \image html MAXREFDES117_Block_Diagram.png "MAXREFDES117# System Block Diagram"
* 
* \image html MAXREFDES117_firmware_Flowchart.png "MAXREFDES117# Firmware Flowchart"
*
*/
#include "Energia.h"
#include "algorithm.h"
#include "max30102.h"
#include "LCD_Launchpad.h"

#define LED 4 //onboard start led indicator
#define MAX_BRIGHTNESS 255

LCD_LAUNCHPAD myLCD;

uint16_t aun_ir_buffer[100]; //infrared LED sensor data
uint16_t aun_red_buffer[100];  //red LED sensor data
int32_t n_ir_buffer_length; //data length
int32_t n_spo2;  //SPO2 value
int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;
String s_n_spo2 = "   ";
String s_n_heart_rate = "   ";
String result = "      ";

// the setup routine runs once when you press reset:
void setup() {
  pinMode(LED, OUTPUT);
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  maxim_max30102_reset(); //resets the MAX30102
  pinMode(5, INPUT);  //pin D10 connects to the interrupt output pin of the MAX30102
  pinMode(24, INPUT_PULLUP);  //start button
  pinMode(25, INPUT_PULLUP);  //start button
  delay(1000);
  //digitalWrite causes the serial.print break, so I comment it. I am not sure why is it happen.
//  digitalWrite(LED, LOW); //led LOW, not running
  maxim_max30102_read_reg(REG_INTR_STATUS_1,&uch_dummy);  //Reads/clears the interrupt status register
  myLCD.init();
  myLCD.clear();
  myLCD.println("press");
//  while(Serial.available()==0)  //wait until user enter any key on serial monitor
//  {
//    Serial.write(27);       // ESC command
//    Serial.print(F("[2J"));    // clear screen command
//    Serial.println(F("Press any key to start conversion"));
//    delay(1000);
//  }
  while(digitalRead(24) == 1 && digitalRead(25) == 1);  //wait until user press button P1.2 or P2.6
//  digitalWrite(LED, HIGH);  //led HIGH, running
  uch_dummy=Serial.read();
  maxim_max30102_init();  //initialize the MAX30102
  myLCD.clear();
  myLCD.println(" hr sp");  //hr: heart rate, sp: SpO2 level
  delay(500);
  myLCD.clear();
}

//display data on led
void display() {
  //if heart rate value changed
  if(!(s_n_heart_rate.equals(String(n_heart_rate)))) {
  //if the changed heart rate value is valid
    if(ch_hr_valid) {
      //changed display heart rate
      s_n_heart_rate = String(n_heart_rate);
      switch(s_n_heart_rate.length()) {
      case 1:
        s_n_heart_rate = "  " + s_n_heart_rate;
        break;
      case 2:
        s_n_heart_rate = " " + s_n_heart_rate;
        break;
      case 3:
        break;
      default:
        s_n_heart_rate = "   ";
      }
    } else {
      //if the changed heart rate value is not valid
      s_n_heart_rate = "inv"; //inv means "invalid"
    }
    //concat heart rate and SpO2 value for display purpose
    result = s_n_heart_rate + s_n_spo2;
    //clear and display new heart rate value
    myLCD.clear();
    myLCD.println(result);
  }
  //if SpO2 value changed
  if(!(s_n_spo2.equals(String(n_spo2)))) {
  //if the changed SpO2 value is valid
    if(ch_spo2_valid) {
      s_n_spo2 = String(n_spo2);
      switch(s_n_spo2.length()) {
      case 1:
        s_n_spo2 = "  " + s_n_spo2;
        break;
      case 2:
        s_n_spo2 = " " + s_n_spo2;
        break;
      case 3:
        break;
      default:
        s_n_spo2 = "   ";
      }
    } else {
      //if the changed SpO2 value is not valid
      s_n_spo2 = "inv"; //inv means "invalid"
    }
    //concat heart rate and SpO2 value for display purpose
    result = s_n_heart_rate + s_n_spo2;
    //clear and display new SpO2 value
    myLCD.clear();
    myLCD.println(result);
  }
}

// the loop routine runs over and over again forever:a
void loop() {
  uint32_t un_min, un_max, un_prev_data, un_brightness;  //variables to calculate the on-board LED brightness that reflects the heartbeats
  int32_t i;
  float f_temp;
  
  un_brightness=0;
  un_min=0x3FFFF;
  un_max=0;
  
  n_ir_buffer_length=100;  //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for(i=0;i<n_ir_buffer_length;i++)
  {
    myLCD.println(" hr sp");
    while(digitalRead(5)==1);  //wait until the interrupt pin asserts
    maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));  //read from MAX30102 FIFO
    
    if(un_min>aun_red_buffer[i])
      un_min=aun_red_buffer[i];  //update signal min
    if(un_max<aun_red_buffer[i])
      un_max=aun_red_buffer[i];  //update signal max
    Serial.print(F("red="));
    Serial.print(aun_red_buffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(aun_ir_buffer[i], DEC);
  }
  
  un_prev_data=aun_red_buffer[i];
  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while(1)
  {
    i=0;
    un_min=0x3FFFF;
    un_max=0;

    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for(i=25;i<100;i++)
    {
      aun_red_buffer[i-25]=aun_red_buffer[i];
      aun_ir_buffer[i-25]=aun_ir_buffer[i];

      //update the signal min and max
      if(un_min>aun_red_buffer[i])
        un_min=aun_red_buffer[i];
      if(un_max<aun_red_buffer[i])
        un_max=aun_red_buffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for(i=75;i<100;i++)
    {
      un_prev_data=aun_red_buffer[i-1];
      while(digitalRead(5)==1);
      maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));

      //calculate the brightness of the LED
      if(aun_red_buffer[i]>un_prev_data)
      {
        f_temp=aun_red_buffer[i]-un_prev_data;
        f_temp/=(un_max-un_min);
        f_temp*=MAX_BRIGHTNESS;
        f_temp=un_brightness-f_temp;
        if(f_temp<0)
          un_brightness=0;
        else
          un_brightness=(int)f_temp;
      }
      else
      {
        f_temp=un_prev_data-aun_red_buffer[i];
        f_temp/=(un_max-un_min);
        f_temp*=MAX_BRIGHTNESS;
        un_brightness+=(int)f_temp;
        if(un_brightness>MAX_BRIGHTNESS)
          un_brightness=MAX_BRIGHTNESS;
      }

      //send samples and calculation result to terminal program through UART
      Serial.print(F("red="));
      Serial.print(aun_red_buffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(aun_ir_buffer[i], DEC);
      
      Serial.print(F(", HR="));
      Serial.print(n_heart_rate, DEC);
      
      Serial.print(F(", HRvalid="));
      Serial.print(ch_hr_valid, DEC);
      
      Serial.print(F(", SPO2="));
      Serial.print(n_spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(ch_spo2_valid, DEC);
    }
    display();
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
  }
}
 
