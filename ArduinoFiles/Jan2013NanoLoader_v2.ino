#define NANO_RX 0
#define NANO_TX 1

#define NANO_RST_N 2
#define NANO_RUN_N 3
#define NANO_TPA_P 4
#define NANO_MRD_N 5
#define NANO_MWR_N 6

#define DATA_OE 7
#define DATA_CLK 8
#define DATA_SER 9

#define ADDR_OE 10
#define ADDR_CLK 11
#define ADDR_SER 12

#define NANO_A16 13
#define NANO_A17 14
#define NANO_ACT_P 15

#define ADDR_S0 16
#define ADDR_S1 17
#define ADDR_RESET_N 18

#define DATA_S0 19
#define DATA_S1 20
#define DATA_RESET_N 21

#define SERIAL_BAUD 115200

#define OE_DISABLE HIGH
#define OE_ENABLE LOW

#define NANO_ENABLE HIGH
#define NANO_DISABLE LOW

#define OK '0'
#define BAD '9'

int intFunction = 0;
unsigned int uintCurrentAddress = 0;
byte bytBuffer[512];
int intBytesRead;
int intPacketLength;
byte bytCurrentPage = 0;

void setPageAddress(byte bytPageAddress) {
  digitalWrite(NANO_A16, bitRead(bytPageAddress,0));
  digitalWrite(NANO_A17, bitRead(bytPageAddress,1));
}

void ADDRReset() {
  digitalWrite(ADDR_RESET_N, LOW);
  delayMicroseconds(30);
  digitalWrite(ADDR_RESET_N, HIGH);
  delayMicroseconds(30);
}

void setADDRMode_DoNothing() {
  digitalWrite(ADDR_S0, LOW);
  digitalWrite(ADDR_S1, LOW);
  addressClockPulse();
  
}

void setADDRMode_ParallelLoad() {
  digitalWrite(ADDR_S0, HIGH);
  digitalWrite(ADDR_S1, HIGH);
  addressClockPulse();
}

void setADDRMode_ShiftRight() {
  digitalWrite(ADDR_S0, HIGH);
  digitalWrite(ADDR_S1, LOW);
  addressClockPulse();
}




void DATAReset() {
  digitalWrite(DATA_RESET_N, LOW);
  delayMicroseconds(30);
  digitalWrite(DATA_RESET_N, HIGH);
  delayMicroseconds(30);
}

void setDATAMode_DoNothing() {
  digitalWrite(DATA_S0, LOW);
  digitalWrite(DATA_S1, LOW);
  dataClockPulse();
  
}

void setDATAMode_ParallelLoad() {
  digitalWrite(DATA_S0, HIGH);
  digitalWrite(DATA_S1, HIGH);
  dataClockPulse();
}

void setDATAMode_ShiftRight() {
  digitalWrite(DATA_S0, HIGH);
  digitalWrite(DATA_S1, LOW);
  dataClockPulse();
}






void setNanoMode(byte bytState) {
  digitalWrite(NANO_ACT_P, bytState);
}

void setCPUReset() {
  digitalWrite(NANO_RST_N, HIGH);  
  delay(1);
  digitalWrite(NANO_RST_N, LOW);
  delay(1);
  digitalWrite(NANO_RST_N, HIGH);  
}

void setCPURun() {
  digitalWrite(NANO_RUN_N, HIGH);  
  digitalWrite(NANO_RUN_N, LOW);
  digitalWrite(NANO_RUN_N, HIGH);  
}


void setADDR_OE(int intSTATUS) {
  digitalWrite(ADDR_OE, intSTATUS);
}

void setDATA_OE(int intSTATUS) {
  digitalWrite(DATA_OE, intSTATUS);
}

void addressClockPulse() {
  digitalWrite(ADDR_CLK, LOW);
  digitalWrite(ADDR_CLK, HIGH);
  digitalWrite(ADDR_CLK, LOW);
}

void dataClockPulse() {
  digitalWrite(DATA_CLK, LOW);
  digitalWrite(DATA_CLK, HIGH);
  digitalWrite(DATA_CLK, LOW);
}

/* This sketch is designed to work on the Jan 2023 build of the 
 *  1802 system designed by Jeff Truck.  This should be uploaded
 *  to the Arduino Nano Every board which has more digital pins
 *  than its predecessor the Arduino Nano.  The desktop application
 *  NanoWriterV1.exe will communicate with this sketch to upload
 *  binary and/or hex files to the 1802 memory.  The desktop 
 *  application also allows for 'remote' control of the 1802 system
 *  by allowing reset and run modes of operation.
 */
 
/* This sketch was optimized in 2024 to vastly speed up the operations 
 *  of sending bytes to the Nano.  The optimization made use of 
 *  direct pin operations, bypassing the normal api functions
 *  which add safety at the cost of performance.
 */

void setup() {

  Serial.begin(SERIAL_BAUD);

  pinMode(NANO_RX,OUTPUT);
  digitalWrite(NANO_RX,LOW);
  pinMode(NANO_TX,OUTPUT);
  digitalWrite(NANO_TX,LOW);
  
  pinMode(NANO_RST_N, OUTPUT);
  digitalWrite(NANO_RST_N,HIGH);
  pinMode(NANO_RUN_N, OUTPUT);
  digitalWrite(NANO_RUN_N,HIGH);
  pinMode(NANO_TPA_P, OUTPUT);
  digitalWrite(NANO_TPA_P,LOW);
  pinMode(NANO_MRD_N, OUTPUT);
  digitalWrite(NANO_MRD_N,HIGH);
  pinMode(NANO_MWR_N, OUTPUT);
  digitalWrite(NANO_MWR_N,HIGH);

  pinMode(DATA_OE, OUTPUT);
  digitalWrite(DATA_OE,HIGH);
  pinMode(DATA_CLK, OUTPUT);
  digitalWrite(DATA_CLK,LOW);
  pinMode(DATA_SER, OUTPUT);
  digitalWrite(DATA_SER,LOW);

  pinMode(ADDR_OE, OUTPUT);
  digitalWrite(ADDR_OE,HIGH);
  pinMode(ADDR_CLK, OUTPUT);
  digitalWrite(ADDR_CLK,LOW);
  pinMode(ADDR_SER, OUTPUT);
  digitalWrite(ADDR_SER,LOW);
  
  pinMode(NANO_A16, OUTPUT);
  digitalWrite(NANO_A16,LOW);
  pinMode(NANO_A17, OUTPUT);
  digitalWrite(NANO_A17,LOW);
  pinMode(NANO_ACT_P, OUTPUT);
  digitalWrite(NANO_ACT_P,LOW);

  pinMode(ADDR_S0, OUTPUT);
  digitalWrite(ADDR_S0, LOW);
  pinMode(ADDR_S1, OUTPUT);
  digitalWrite(ADDR_S1, LOW);
  pinMode(ADDR_RESET_N, OUTPUT);
  digitalWrite(ADDR_RESET_N, HIGH);

  pinMode(DATA_S0, OUTPUT);
  digitalWrite(DATA_S0, LOW);
  pinMode(DATA_S1, OUTPUT);
  digitalWrite(DATA_S1, LOW);
  pinMode(DATA_RESET_N, OUTPUT);
  digitalWrite(DATA_RESET_N, HIGH);

  delay(1000);

  setCPUReset();
  
}

void loop() {

  /*
   * command codes:
   * 1 : Set Current Address
   * 2 : Program Bytes to Memory
   * 3 : Set Current Page Address
   * 4 : Do Nothing
   * 5 : Enter Programming Mode
   * 6 : Exit Programming Mode
   * 7 : Set CPU RESET
   * 8 : Set CPU RUN
   * 9 : Do Nothing
   */

  // set serial timeout to 5 seconds...
  //Serial.setTimeout(5);
  
  while(1) {

    if (Serial.available() > 0) {
      intBytesRead = Serial.readBytes(bytBuffer,1);
      if (intBytesRead == 1) {
        intFunction = bytBuffer[0];        
      }
      else {
        intFunction = 0;
        Serial.write(BAD);
      }
    }
    else {
      intFunction = 0;
    }

    if (intFunction > 0) {

      switch (intFunction) {

        case '1':   // Set Current Address

          intBytesRead = Serial.readBytes(bytBuffer,2);
          
          if (intBytesRead == 2) {
            uintCurrentAddress = (bytBuffer[0]*256)+bytBuffer[1];
            Serial.write(OK);
          }
          else {
            Serial.write(BAD);
          }
          
          break;
        
        case '2':   // Program Bytes to Memory

          setNanoMode(HIGH);
          
          intBytesRead = Serial.readBytes(bytBuffer,2);

          if (intBytesRead == 2) {
            
            intPacketLength = (bytBuffer[0]*256)+bytBuffer[1];

            intBytesRead = Serial.readBytes(bytBuffer,intPacketLength);

            if (intBytesRead == intPacketLength) {

              int intLCV = 0;
              while (intLCV < intPacketLength) {
                byte bytSingle = bytBuffer[intLCV];

                digitalWrite(NANO_RX,HIGH);
                digitalWrite(NANO_RX,LOW);
                int intHighOrder = (uintCurrentAddress >> 8);
                int intLowOrder = ((uintCurrentAddress << 8) >> 8);
                
                for (int intLCV=7; intLCV >= 0; intLCV--) {

                  //digitalWrite(ADDR_SER,bitRead(intHighOrder,intLCV));
                  if ( bitRead(intHighOrder,intLCV) == LOW ) {
                    PORTE.OUTCLR = PIN1_bm;
                  }
                  else {
                    PORTE.OUTSET = PIN1_bm;
                  };
              
                  // Pulse the address clock...
                  //digitalWrite(ADDR_CLK, LOW);
                  //digitalWrite(ADDR_CLK, HIGH);
                  //digitalWrite(ADDR_CLK, LOW);
                  PORTE.OUTCLR = PIN0_bm;
                  PORTE.OUTSET = PIN0_bm;
                  PORTE.OUTCLR = PIN0_bm;
                
                }
              
                // Enable address output...
                digitalWrite(ADDR_OE, LOW);
              
                // pulse the TPA...
                //digitalWrite(NANO_TPA_P, LOW);
                //digitalWrite(NANO_TPA_P, HIGH);
                //digitalWrite(NANO_TPA_P, LOW);
                PORTC.OUTCLR = PIN6_bm;
                PORTC.OUTSET = PIN6_bm;
                PORTC.OUTCLR = PIN6_bm;
                
              
                // Disable address output...
                digitalWrite(ADDR_OE, HIGH);
                
                for (int intLCV=7; intLCV >= 0; intLCV--) {
                  
                  //digitalWrite(ADDR_SER,bitRead(intLowOrder,intLCV));
                  if ( bitRead(intLowOrder,intLCV) == LOW ) {
                    PORTE.OUTCLR = PIN1_bm;
                  }
                  else {
                    PORTE.OUTSET = PIN1_bm;
                  };

                  // Pulse the address clock...
                  //digitalWrite(ADDR_CLK, LOW);
                  //digitalWrite(ADDR_CLK, HIGH);
                  //digitalWrite(ADDR_CLK, LOW);
                  PORTE.OUTCLR = PIN0_bm;
                  PORTE.OUTSET = PIN0_bm;
                  PORTE.OUTCLR = PIN0_bm;                  
                }
              
                // Enable address output...
                digitalWrite(ADDR_OE, LOW);
                              
                //setData(bytData);
                for (int intLCV=7; intLCV >= 0; intLCV--) {    

                  //digitalWrite(DATA_SER,bitRead(bytSingle,intLCV));
                  if ( bitRead(bytSingle,intLCV) == LOW ) {
                    PORTB.OUTCLR = PIN0_bm;
                  }
                  else {
                    PORTB.OUTSET = PIN0_bm;
                  };
                                   
                  // Pulse the data clock...
                  //digitalWrite(DATA_CLK, LOW);
                  //digitalWrite(DATA_CLK, HIGH);
                  //digitalWrite(DATA_CLK, LOW);
                  PORTE.OUTCLR = PIN3_bm;
                  PORTE.OUTSET = PIN3_bm;
                  PORTE.OUTCLR = PIN3_bm;                  
                }
              
                // enable the data output...
                digitalWrite(DATA_OE, LOW);
              
                // pulse MWR
                //digitalWrite(NANO_MWR_N, LOW);
                //digitalWrite(NANO_MWR_N, HIGH);
                PORTF.OUTCLR = PIN4_bm;
                PORTF.OUTSET = PIN4_bm;
                
              
                // disable address and data outputs
                digitalWrite(ADDR_OE, HIGH);
                digitalWrite(DATA_OE, HIGH);
                
                uintCurrentAddress = uintCurrentAddress + 1;
                intLCV = intLCV + 1; 
              }
             
              Serial.write(OK);
                           
            }
            else {
              Serial.write(0x01);
            }            
          }
          else {
            Serial.write(0x02);
          }

          setNanoMode(LOW);
          
          break;

        case '3':   // Set Current Page Address

          intBytesRead = Serial.readBytes(bytBuffer,1);

          if (intBytesRead == 1) {
            bytCurrentPage = bytBuffer[0];
            setPageAddress(bytCurrentPage);
            Serial.write(OK);
          }
          else {
            Serial.write(0x01);
          }            
          
          break;

        case '4':   // Do Nothing
          Serial.write(OK);
          break;

        case '5':   // Enter Programming Mode

          setADDRMode_DoNothing();
          ADDRReset();
          setADDRMode_ShiftRight();

          setDATAMode_DoNothing();
          DATAReset();
          setDATAMode_ShiftRight();

          setPageAddress(0);
                    
          Serial.write(OK);
          break;
          
        case '6':   // Exit Programming Mode

          setADDRMode_DoNothing();
          ADDRReset();
          setADDR_OE(OE_DISABLE);

          setDATAMode_DoNothing();
          DATAReset();
          setDATA_OE(OE_DISABLE);

          setPageAddress(0);

          Serial.write(OK);
          break;

        case '7':   // Set CPU RESET

          setCPUReset();
          Serial.write(OK);
          break;

        case '8':   // Set CPU RUN

          setCPURun();
          Serial.write(OK);
          break;

        case '9':   // Do Nothing
          
          Serial.write(OK);
          break;
          
      }
      
      intFunction = 0;
      
    }   
  }
}
