/*
   Heater 1 is using PWM pin #9
   Heater 2 is using PWM pin #10
   PWM pin #9 and #10 belong to Timer 1.
   Thus, we put out PID in Timer 2.
   Timer 0 can be used but its setting can not be modified since it is reserved by the Arduino for timing purposes.
*/

#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
#define USE_TIMER_2                   true

#include "TimerInterrupt.h"


#define TIMER2_INTERVAL_MS             1


const int pwm1_port       = 9;     // PWM of heater #1, Timer 1A
const int pwm2_port       = 10;    // PWM of heater #2, Timer 1B


float past_err  = 0.0;   // previous error
float err       = 0.0;   // current error
float P         = 0.0;   // proportional term
float I         = 0.0;   // integral term
float D         = 0.0;   // derivative term
float Df        = 0.0;   // filtered derivative term


float KP        = 100.0; // kp
float KD        = 0.01;  // ki
float KI        = 0.1;   // hs
float CO        = 0.0;   // control output
float PV        = 0.0;   // process value
float SV        = 0.0;   // set value


unsigned long base;
unsigned long now;       // current time stamp in micro-secs
unsigned long prev;      // previous time stamp in micro-secs
float dt;                // elapsed time in milli-secs


const byte numChars = 32;
char       receivedChars[numChars];
boolean    newData = false;


void setup() {
  // https://arduinoinfo.mywikis.net/wiki/Arduino-PWM-Frequency
  cli();
  TCCR1B = TCCR1B & B11111000 | B00000010;    // 3921.16 Hz
  sei();

  ITimer2.init();
  ITimer2.attachInterruptInterval(TIMER2_INTERVAL_MS, Timer2Handler);

  Serial.begin(9600);

  pinMode(pwm1_port, OUTPUT);
  pinMode(pwm2_port, OUTPUT);
  analogWrite(pwm1_port, 0);
  analogWrite(pwm2_port, 0);

  base = micros();
  now  = micros() - base;
}


inline void clock_update()
{
  prev = now;
  now  = micros() - base;
  dt   = (float)(now - prev) / 1000.0; // in msecs
}


/*
   Put your PID control here!
*/
float tau = 0.1; // seconds
inline void PID()
{
  PV       = (float)analogRead(A0) * 500.0 / 1024.0;

  past_err = err;
  err      = SV - PV;

  P        = KP * err;
  D        = KD * (err - past_err) / (dt * 1e-3);
  I        = I + KI * (dt * 1e-3) * err;
  Df       = (tau * Df + (dt * 1e-3) * D) / (tau + (dt * 1e-3)) ;

  CO       = P + I + Df;
}


void Timer2Handler()
{
  // Update all time variables
  clock_update();

  // PID control
  PID();

  // Guard the control signal
  if (CO > 255.)
    CO = 255.;
  else if (CO < 0.)
    CO = 0.;

  // Send control output to PWM
  analogWrite(pwm1_port, (int)CO);
}


void loop()
{
  // Serial receive
  rxSV();

  // Serial transmit
  txPV();

  delay(1000);
}


void rxSV() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

  if (newData == true) {
    SV = atof(receivedChars);
    Serial.print("Received new SV: ");
    Serial.println(SV);

    newData = false;
  }
}


inline void txPV()
{
  Serial.print("<");
  Serial.print(PV, 3);
  Serial.print(">");
}
