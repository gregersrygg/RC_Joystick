#include <avr/io.h>
#include <avr/interrupt.h>

// Connect simulator cable to GND and pin 0
const int PIN = 0;
const int CHANNELS = 8;

/* CHANNELS (Turnigy 9x)
  0 unused
  1 roll
  2 pitch
  3 throttle
  4 yaw
  5 aux
*/
const int CH_ROLL  = 1;
const int CH_PITCH = 2;
const int CH_THR   = 3;
const int CH_YAW   = 4;
const int CH_AUX   = 5;


int state = 0;
int channel = 0;
const uint32_t MIN_TIME = 680;
const uint32_t MAX_TIME = 1580;
const uint32_t MAX_DIFF = (MAX_TIME - MIN_TIME);
const uint32_t MAX_VAL = 1023;

volatile uint32_t risingEdge[CHANNELS]; // time of last rising edge for each channel
volatile uint32_t value[CHANNELS];


void setup() {
  Serial.begin(9600);
  Serial.print("setup");

  
  pinMode(PIN, INPUT);
  
  attachInterrupt(PIN, pinChange, CHANGE); // one or more of pins 2~7 have changed state

  Joystick.useManualSend(true);
}

uint32_t uSecToValue(float uSec) {
  return min(MAX_VAL, max(0, (uSec - MIN_TIME) / MAX_DIFF * MAX_VAL));
}

void pinChange() {
  state = digitalRead(PIN);
  uint32_t now = micros();
  
  if (state == HIGH) {
    channel++;
    risingEdge[channel] = now;
  } else {
    uint32_t uSec = now - risingEdge[channel];
    if (uSec > 2000) {
      channel = 0;
    } else {
      value[channel] = uSecToValue(uSec);
    }
    
  }
  
  if (channel >= CHANNELS) {
    channel = 0;
  }
  
}

void loop() {
  Serial.flush();
  Serial.print("PPM ");
  for (int channel = 0; channel < CHANNELS; channel++) {
    Serial.print(value[channel]);
    Serial.print("\t");
  }
  Serial.println();
  
  Joystick.X(value[CH_ROLL]);
  Joystick.Y(value[CH_PITCH]);
  Joystick.Z(value[CH_THR]);
  Joystick.Zrotate(value[CH_YAW]);
  int aux = (value[CH_AUX] > 512 ? 1 : 0);
  Joystick.button(1, aux);
  Joystick.send_now();
  
  // a short delay seems to help some applications to accept the joystick
  delay(50);
}
