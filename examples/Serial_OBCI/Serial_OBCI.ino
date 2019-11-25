//#define ADS129X_POLLING

#include "ADS129X.h"
#include <SPI.h>

//check if this is a Teensy 4.0 board
#if defined(__IMXRT1062__)
extern "C" uint32_t set_arm_clock(uint32_t frequency);
#endif


/* ADS129X pins for Biopotential Library reference hardware https://oshpark.com/shared_projects/FCVCtANz */
const int ADS_PWDN = 3;
const int ADS_RESET  = 5;
const int ADS_START  = 6;
const int ADS_DRDY = 2;
const int ADS_DRDY2 = 1;
const int ADS_DRDY3 = 0;
const int ADS_DRDY4 = 4;
const int ADS_CS = 10;
const int ADS_CS2 = 19;
const int ADS_CS3 = 18;
const int ADS_CS4 = 17;

ADS129X* ADS = new ADS129X(ADS_DRDY, ADS_CS);
//ADS129X* ADS2 = new ADS129X(ADS_DRDY2, ADS_CS2);
//ADS129X* ADS3 = new ADS129X(ADS_DRDY3, ADS_CS3);

//long average[10];
long avg;   //Computed average of 8 values.
long zero[1000];
long zero1[1000];
long zeroComp;  //Computed average of 1000 values for zero offset.
long zeroComp1;  //Computed average of 1000 values for zero offset.
static unsigned long tLast = 0L;


void setup() {
  pinMode(ADS_PWDN, OUTPUT);
  pinMode(ADS_RESET, OUTPUT);
  pinMode(ADS_START, OUTPUT);

//    pinMode(ADS_CS2, OUTPUT);
//    digitalWriteFast(ADS_CS2, HIGH);
//        pinMode(ADS_CS3, OUTPUT);
//    digitalWriteFast(ADS_CS3, HIGH);
//        pinMode(ADS_CS4, OUTPUT);
  
  digitalWriteFast(ADS_START, HIGH);
  digitalWriteFast(ADS_PWDN, HIGH);
  digitalWriteFast(ADS_RESET, HIGH);
  delay(100); // delay for power-on-reset (Datasheet, pg. 48)
  // reset pulse
  digitalWriteFast(ADS_RESET, LOW);
  digitalWriteFast(ADS_RESET, HIGH);
  delay(1); // Wait for 18 tCLKs AKA 9 microseconds, we use 1 millisec

  ADS->SDATAC(); // device wakes up in RDATAC mode, so send stop signal
//  ADS->WREG(ADS129X_REG_CONFIG1, 
//                              (1<<7) |  // HR on ADS1298
// //                             (1<<6) |  // Daisy off = 1
//                              ADS129X_SAMPLERATE_16k
//                                 ); // enable sample-rate for ADS1298
  ADS->WREG(ADS129X_REG_CONFIG1, 
                              (1<<7) |  // always 1 on ADS1299
 //                             (1<<6) |  // Daisy off = 1
                              (1<<4) |  // always 1 for ADS1299
                              ADS1299_SAMPLERATE_16k
                                 ); // enable sample-rate for ADS1299
  ADS->WREG(ADS129X_REG_CONFIG3, (1<<ADS129X_BIT_PD_REFBUF)   // enable internal reference
                              | (1<<6)                       // always 1
                              | (1<<5)                       // always 1 for ADS1299, VREF_4V = 1 for ADS1298
                              | (1<<ADS129X_BIT_RLD_MEAS)    // BIAS_IN signal is routed to the channel that has the MUX_Setting 010 (VREF)
                              | (1<<ADS129X_BIT_RLDREF_INT)  //  BIASREF signal (AVDD + AVSS)/2 generated internally
                              | (1<<ADS129X_BIT_PD_RLD)      //BIAS buffer is enabled
                              ); 
  //ADS.WREG(ADS129X_REG_CONFIG2, (1<<ADS129X_BIT_INT_TEST) | ADS129X_TEST_FREQ_2HZ);
    ADS->WREG(ADS129X_REG_RLD_SENSP, (1<<ADS129X_BIT_CH1)
//                                  | (1<<ADS129X_BIT_CH2)
                                  ); // Route channel n positive signal into BIAS channel
    ADS->WREG(ADS129X_REG_RLD_SENSN, (1<<ADS129X_BIT_CH1)
//                                  | (1<<ADS129X_BIT_CH2)
                                  ); // Route channel n negative signal into BIAS channel
  // setup channels
  for (int i = 1; i <= 8; i++) {
//    ADS->configChannel(i, false, ADS129X_GAIN_12X, ADS129X_MUX_NORMAL); //Gain for ADS1298
    ADS->configChannel(i, false, ADS1299_GAIN_24X, ADS129X_MUX_NORMAL);  //Gain for ADS1299
  }

//  delay(1);
  ADS->RDATAC();
  ADS->START();

Serial.begin(12000000); // always at 12Mbit/s
#if defined(__IMXRT1062__)  //Downclock Teensy 4.0 to prevent overheating
//    set_arm_clock(400000000);
#endif
}

//Convert signed 24 bit to signed 32 bit
long toInt32(long rVal) 
{
  long result = rVal & 0x800000 ? 0xff000000 | rVal : rVal;
  return result;
}

//Return voltage in nanovolts.  ADS1299 voltage range is internally set to 4.5V
// 2^24/1000000000 = 0.016777216
double getnVolts(int val){
  double volts = (((double)val)*4.5)/(24.0 * 0.016777216);
  return volts;
}

void loop() {
  long buffer[9];

  if (ADS->getData(buffer)) {
    avg = 0;
    zeroComp = 0;
    zeroComp1 = 0;
    for(int i=1; i<=8; i++){
      avg += toInt32(buffer[i]);
    }
    for(int i=0; i<999; i++){
      zeroComp += zero[i];
      zeroComp1 += zero1[i];
      zero[i]=zero[i+1];
      zero1[i]=zero1[i+1];
    }
    zeroComp += zero[999];
    zeroComp1 += zero1[999];
    avg = avg/8;
    zero[999]=avg;
    zero1[999]=toInt32(buffer[1]);
    zeroComp = zeroComp/1000;
    zeroComp1 = zeroComp1/1000;

//Print in format for Serial Plotter
//        Serial.print(getnVolts(toInt32(buffer[1]) - zeroComp1));
//        Serial.print(" ");
     Serial.print(getnVolts(avg - zeroComp));
#if defined(__IMXRT1062__)  //Downclock Teensy 4.0 to prevent overheating
     Serial.print(" F_CPU_ACTUAL=");
     Serial.print(F_CPU_ACTUAL);
#endif
     Serial.print(" OffsetMicroVolts=");
     Serial.print(getnVolts(zeroComp)/1000.0);
     Serial.print(" TimeMicros=");
     Serial.print(micros()-tLast);
     Serial.println("");
     tLast = micros();
  }
}
