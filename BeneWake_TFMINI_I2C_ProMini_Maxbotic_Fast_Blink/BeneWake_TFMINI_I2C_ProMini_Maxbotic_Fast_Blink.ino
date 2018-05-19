/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.


   This is a MaxBotix MaxsonarI2CXL emulator program that can be used with a
   Benewake TFMINI TOF Laser rangefinder. This allows the serial stream to be converted
   to an I2C message that is more convenient for most Flight Controler that are lacking 
   UARTS. 
   Tested with Ardupilot by Patrick Poirier December 2017.
   Might containe open licenced code from other sources, thank you ;-)
   More on my github https://github.com/patrickpoirier51
 */


#include <Wire.h>
#define I2C_SLAVE_ADDR  0x70  //Standard MaxsonarI2CXL address 0x70 

uint8_t LED = 13;
uint8_t Framereceived[9]; //TFMINI received Buffer
uint8_t Index;
byte received;
uint16_t distance;      // Distance value in 16 bit word
uint8_t OffSet = 120 ;  //Offset error of the LIDAR
uint8_t Scaling = 10 ;  //Benewake outputs in mm, we have to scale to cm
uint16_t strength;
uint16_t strLimit = 10;
uint16_t distLow = 46; 
uint16_t OutdoorMax = 700;

//Checksum is the sum of the first 8 bytes of actual data Checksum = Byte1 + Byte2 + ... + Byte8
uint8_t Checksum(uint8_t *data, uint8_t length)
 {
     uint16_t  count;
     uint16_t  Sum = 0;
     
     for (count = 0; count < length; count++)
         Sum = Sum + data[count];
     return (Sum); 
 }

/* 
This is the structure of the Benewake TFMINI serial message
Byte1-2   Byte3   Byte4   Byte5       Byte6        Byte7      Byte8     Byte9
0x59 59   Dist_L  Dist_H  Strength_L  Strength_H   Reserved   Raw.Qual  CheckSum_
*/

void readlaser(){     
  if (Serial.available() > 0) {
      uint8_t inChar = Serial.read();
       if((inChar=='Y')&& (Index==0)){
        Framereceived[Index]=inChar;
        Index++;
       }
       else{
         if( Framereceived[0]=='Y'){
          if(Index<8){
            Framereceived[Index]=inChar;
            Index++;
          }
          else{
             Framereceived[Index]=inChar;
             if( Framereceived[1]=='Y'){
              if(Checksum(Framereceived, 8)==Framereceived[8]){
              digitalWrite(LED, !digitalRead(LED));  //toggle LED
              distance= (uint16_t)(((Framereceived[2] + (Framereceived[3]*256))+OffSet)/Scaling);
              strength = (uint16_t)(Framereceived[4] + (Framereceived[5]*256));
              if ( (strength < strLimit) && (distance <= distLow ))  {
                  distance = OutdoorMax;
                    }  
               }  
               }           
             for(uint8_t i=0;i<8;i++){
             Framereceived[i]=0;
             }
            Index=0;
          }
       }
    }
  }

}

/*
 The default I2C-Address of the sensor is 0x70. 
 To perform a range measurement you must send the "Take Range Reading” command byte 0x5
 */
void receiveEvent(int howMany) {
  while (Wire.available()) { // loop through all but the last
        {         
      received = Wire.read(); 
      if (received == 0x51)
      {
      //Serial.println(received);        
      }
    }
  }
}

void requestEvent() 
{
Wire.write (highByte(distance));
Wire.write (lowByte(distance));
}

void setup() {
 pinMode (LED, OUTPUT);
 digitalWrite(LED, LOW);
 Serial.begin(115200);
 Wire.begin(I2C_SLAVE_ADDR);
 Wire.onReceive(receiveEvent); // register event
 Wire.onRequest(requestEvent);
}


void loop() {
  
  readlaser();
  for (int i=0; i<100; i++){

}


