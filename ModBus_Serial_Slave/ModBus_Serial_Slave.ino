
#include <Modbus.h>
#include <ModbusSerial.h>
//#include "functions.h"
#include <avr/pgmspace.h>
#include "q3.h"
#include "cl1.h"
#include "cond1.h"
#include <Wire.h>

#define PRINTREGISTERS

// ModBus Port information
#define BAUD 9600
#define ID 1             //MODBUS SLAVE ID
#define TXPIN 5          //send/receive enable for RS485
#define VLC_STR_LEN 100  //MAX number of characters to send in VLC data.

ModbusSerial modbus;

const int ledPin = RED_LED;

unsigned long update_time = 0;
unsigned long inc_time = 0;

#define LAMP_I2C_ADDR 0x20
#define LAMP_BR_ADDR1 0x14
#define LAMP_BR_ADDR1 0x15


//INPUT REGISTER ADDRESSES
#define TEMP_IP 3000
#define TEMP_PIN A6             // connected to PD1 - A6
#define VOLTAGE_IP 3001
#define VOLTAGE_PIN A1          //connected to PE2 - A1
#define CURRENT_IP 3002
#define CURRENT_PIN A0          //connected to PE3 - A0
#define LDR_IP 3003
#define LDR_PIN A5              //connected to PD2 - A5

//HOLDING REGISTER ADDRESSES
#define VLC_START_H 1000
#define BRIGHTNESS_H 2000
#define CHIP_TEMP_H 3000


byte ioconf1[2]={0x00,0x00};  //GPIO expander configs
byte ioconf2[2]={0x01,0x00};


//BRIGHTNESS ENCODED VALUES staring from 0 to 100 in steps of 10
byte data1[11][2]={{0x14,0xF0},{0x14,0xF0},{0x14,0xF0},{0x14,0x70},{0x14,0x70},{0x14,0x70},{0x14,0x70},{0x14,0x80},{0x14,0x00},{0x14,0x00},{0x14,0x00}};
byte data2[11][2]={{0x15,0xF9},{0x15,0xF1},{0x15,0xC9},{0x15,0xB8,},{0x15,0xB0},{0x15,0x88},{0x15,0x80},{0x15,0x41},{0x15,0x30},{0x15,0x08},{0x15,0x00}};


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  // Config Modbus Serial (port, speed, byte format)
  modbus.config(&Serial1, BAUD, TXPIN); // Change to Serial1 before deployment

  // Set the Slave ID (1-247)
  modbus.setSlaveId(ID);

//  // Use addIsts() for digital inputs - Discrete Input - Master Read-Only

//    modbus.addIsts(i);

//  // Use addIreg() for analog Inputs - Input Register - Master Read-Only

//    modbus.addIreg(i);

//  // Use addCoil() for digital outputs -  Coil - Master Read-Write
//    modbus.addCoil(i);

//
//  // Use addHreg() for analog outpus - Holding Register - Master Read-Write
//    modbus.addHreg(i, i)



  //CREATE INPUT REGISTERS
  modbus.addIreg(TEMP_IP, 0);
  modbus.addIreg(VOLTAGE_IP, 0);
  modbus.addIreg(CURRENT_IP, 0);
  modbus.addIreg(LDR_IP, 0);


  //CREATE HOLDING REGISTERS

  modbus.addHreg(BRIGHTNESS_H, 20);
  modbus.addHreg(CHIP_TEMP_H, 0);

  for(int i=VLC_STR_LEN; i<(VLC_STR_LEN+VLC_START_H); i++)
      modbus.addHreg(i, '\0');
  Wire.setModule(1);
  Wire.begin();
  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(ioconf1, 2);
  Wire.endTransmission();


  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(ioconf2, 2);
  Wire.endTransmission();

  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(data1[5], 2);
  Wire.endTransmission();


  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(data2[5], 2);
  Wire.endTransmission();

  modbus.Hreg(BRIGHTNESS_H, 50);
}


int oldb=0;


void loop() {
  //Call once inside loop() - all magic here


  if(modbus.task())
  {
      //UPDATE BRIGHTNESS
      //UPDATE BRIGHTNESS OF LAMP
        int b=modbus.Hreg(BRIGHTNESS_H);
        b=(b/10);
        if (b>10) b=10;
        if(b<0) b=0;
        if(oldb!=b){

        oldb=b;

        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data1[b], 2);
        Wire.endTransmission();


        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data2[b], 2);
        Wire.endTransmission();
        }


      //UPDATE VLC STRING
      int i=VLC_START_H;
      while(modbus.Hreg(i)!='\0')
      {

      i++;
      }//ALSO ENCODE STRING


  }

  //SEND ENCODED VLC STRING


  if (millis() > update_time + 500) {
      update_time = millis();
      modbus.Ireg(LDR_IP, analogRead(LDR_PIN));
      modbus.Ireg(TEMP_IP, analogRead(TEMP_PIN));
      modbus.Ireg(VOLTAGE_IP, analogRead(VOLTAGE_PIN));
      modbus.Ireg(CURRENT_IP, analogRead(CURRENT_PIN));
      modbus.Hreg(CHIP_TEMP_H, analogRead(TEMPSENSOR));


  }
}























//  if (millis() > inc_time + 500) {
//    inc_time = millis();
//
//    ++value;
//
//    if(value >= 1000)
//      value = 0;
//  }
//
//  if (millis() > update_time + 1000) {
//    update_time = millis();
//
//    // Set Digital Input - Discrete Input - Master Read-Only
//    modbus.Ists(0, HIGH);
//
//    // Set Analog Input - Input Register - Master Read-Only
//    modbus.Ireg(0, 125);
//    modbus.Ireg(1, analogRead(A0));
//
//    //Attach ledPin to Digital Output - Coil - Master Read-Write 
//    //modbus.Coil(0, !modbus.Coil(0));
//    digitalWrite(ledPin, modbus.Coil(0));
//
//    // Write to Analog Output - Holding Register - Master Read-Write
//    modbus.Hreg(0, value);
//    modbus.Hreg(4, 99);
//
//    float value_q3 = pgm_read_float_near(q3 + idx);
//    float value_cl1 = pgm_read_float_near(cl1 + idx);
//    float value_cond1 = pgm_read_float_near(cond1 + idx);
//
//    modbus.Hreg(0x55,value_q3*10);
//    modbus.Hreg(0x57,value_cl1*1000);
//    modbus.Hreg(0x59, value_cond1*100);
//
//    ++idx;
//    if( idx == 1124)
//      idx = 0;
//  }
//
//}

