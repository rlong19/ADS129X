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
const int ADS_START  = 4;
const int ADS_DRDY = 2;
const int ADS_CS = 20;

ADS129X ADS = ADS129X(ADS_DRDY, ADS_CS);

void setup() {
  pinMode(ADS_PWDN, OUTPUT);
  pinMode(ADS_RESET, OUTPUT);
  pinMode(ADS_START, OUTPUT);

  
  digitalWrite(ADS_START, HIGH);
    digitalWrite(ADS_PWDN, HIGH);
  digitalWrite(ADS_RESET, HIGH);
  delay(100); // delay for power-on-reset (Datasheet, pg. 48)
  // reset pulse
  digitalWrite(ADS_RESET, LOW);
  digitalWrite(ADS_RESET, HIGH);
  delay(1); // Wait for 18 tCLKs AKA 9 microseconds, we use 1 millisec

  ADS.SDATAC(); // device wakes up in RDATAC mode, so send stop signal
  ADS.WREG(ADS129X_REG_CONFIG1, ADS1299_SAMPLERATE_250); // enable 1kHz sample-rate
  ADS.WREG(ADS129X_REG_CONFIG3, (1<<ADS129X_BIT_PD_REFBUF)   // enable internal reference
                              | (1<<6)                       // always 1
                              | (1<<ADS129X_BIT_RLD_MEAS)    // BIAS_IN signal is routed to the channel that has the MUX_Setting 010 (VREF)
                              | (1<<ADS129X_BIT_RLDREF_INT)  //  BIASREF signal (AVDD + AVSS)/2 generated internally
                              | (1<<ADS129X_BIT_PD_RLD)      //BIAS buffer is enabled
                              ); 
  //ADS.WREG(ADS129X_REG_CONFIG2, (1<<ADS129X_BIT_INT_TEST) | ADS129X_TEST_FREQ_2HZ);
    ADS.WREG(ADS129X_REG_RLD_SENSP, (1<<ADS129X_BIT_CH1)
//                                  | (1<<ADS129X_BIT_CH2)
                                  ); // Route channel n positive signal into BIAS channel
    ADS.WREG(ADS129X_REG_RLD_SENSN, (1<<ADS129X_BIT_CH1)
//                                  | (1<<ADS129X_BIT_CH2)
                                  ); // Route channel n negative signal into BIAS channel
  // setup channels
  ADS.configChannel(1, false, ADS1299_GAIN_24X, ADS129X_MUX_NORMAL);
//  ADS.configChannel(2, false, ADS1299_GAIN_24X, ADS129X_MUX_NORMAL);
//  ADS.configChannel(3, false, ADS1299_GAIN_24X, ADS129X_MUX_NORMAL);
//  ADS.configChannel(4, false, ADS1299_GAIN_24X, ADS129X_MUX_NORMAL);
  for (int i = 2; i <= 8; i++) {
    ADS.configChannel(i, true, ADS129X_GAIN_1X, ADS129X_MUX_SHORT);
//    ADS.configChannel(i, false, ADS1299_GAIN_24X, ADS129X_MUX_NORMAL);
  }

  delay(1);
  ADS.RDATAC();
  ADS.START();

  Serial.begin(1000000); // always at 12Mbit/s
  #if defined(__IMXRT1062__)  //Downclock Teensy 4.0 to prevent overheating
    set_arm_clock(100000000);
    Serial.print("F_CPU_ACTUAL=");
    Serial.println(F_CPU_ACTUAL);
#endif
  Serial.println("Firmware v0.0.1");
}

void loop() {
  long buffer[9];
  static unsigned long tLast;
  if (millis()-tLast > 500) {
//    digitalWrite(LED3, !digitalRead(LED3));
    tLast = millis();
  }
  if (ADS.getData(buffer)) {
    unsigned long tStart = micros();
    for (int channel = 1; channel < 2; channel++) {
      /*if (channel == 8) {
        buffer[channel] = micros()-tStart;
      }*/

//Print in format for Serial Plotter
        Serial.print(buffer[channel]);
//        Serial.print(" ");
//        Serial.print(buffer[channel+1]);
#if defined(__IMXRT1062__)  //Downclock Teensy 4.0 to prevent overheating
        Serial.print(" F_CPU_ACTUAL=");
        Serial.print(F_CPU_ACTUAL);
#endif
        Serial.println("");

    }
  }
}
