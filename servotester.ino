#include <Servo.h>
#include <U8x8lib.h>

#define SERVO_COUNT   8
#define SERVO_MIN     1000
#define SERVO_MAX     2000
#define SERVO_DEFAULT 1500
#define ENC_SW_PIN    15
#define ENC_A_PIN     2
#define ENC_B_PIN     3
#define DISP_COLS     16

Servo servos[SERVO_COUNT];
int servoPins[SERVO_COUNT] = {17, 16, 4, 5, 6, 7, 8, 9};
int servoValues[SERVO_COUNT];
int servoPtr = 0;
int servoSel = -1;

volatile int lastEncoded = 0;
int lastEncSw = HIGH;

U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);
char dispbuf[DISP_COLS];

void setup() {
  disp.begin();
  disp.setFont(u8x8_font_pxplustandynewtv_f);
  disp.clear();

  pinMode(ENC_A_PIN, INPUT);
  pinMode(ENC_B_PIN, INPUT);
  pinMode(ENC_SW_PIN, INPUT);
  digitalWrite(ENC_A_PIN, HIGH);
  digitalWrite(ENC_B_PIN, HIGH);
  digitalWrite(ENC_SW_PIN, HIGH);
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);

  for (int i = 0; i < SERVO_COUNT; i++) {
    servos[i].attach(servoPins[i]); 
    servoValues[i] = SERVO_DEFAULT;
    servos[i].writeMicroseconds(servoValues[i]);
  }
}

void updateEncoder(){
  int MSB = digitalRead(ENC_A_PIN);
  int LSB = digitalRead(ENC_B_PIN);
  
  int encoded = (MSB << 1) | LSB;
  int sum  = (lastEncoded << 2) | encoded;

  if (servoSel == -1) {
    if(sum == 0b1101) servoPtr++;
    if(sum == 0b1110) servoPtr--;

    if (servoPtr < 0) servoPtr = SERVO_COUNT - 1;
    if (servoPtr >= SERVO_COUNT) servoPtr = 0;
  }
  else {
    if(sum == 0b1101) servoValues[servoSel] += 10;
    if(sum == 0b1110) servoValues[servoSel] -= 10;

    if (servoValues[servoSel] < SERVO_MIN) servoValues[servoSel] = SERVO_MIN;
    if (servoValues[servoSel] > SERVO_MAX) servoValues[servoSel] = SERVO_MAX;
    servos[servoSel].writeMicroseconds(servoValues[servoSel]);
  }
  lastEncoded = encoded;
}

void loop() {
  int encSw = digitalRead(ENC_SW_PIN);
  if (lastEncSw != encSw) {
    if (lastEncSw == LOW && encSw == HIGH) {
      if (servoSel == -1) {
        servoSel = servoPtr;
      }
      else {
        servoSel = -1;
      }
    }
    lastEncSw = encSw;
  }

  for (int j = 0; j < 2; j++) {    
    for (int i = 0; i < SERVO_COUNT / 2; i++) {    
      int idx = (j * SERVO_COUNT / 2) + i;
      snprintf(dispbuf, DISP_COLS, "%s%d:%d", (servoSel < 0 ? (idx == servoPtr ? "*" : " ") : (idx == servoSel ? ">" : " ")), idx + 1, servoValues[idx]);
      disp.drawString(j * 8, i, dispbuf);    
    }
  }
  
}
