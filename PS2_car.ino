#include <PS2X_lib.h> //Thư viện tay điều khiển
#include <ESP32MX1508.h> //Thư viện điều khiển động cơ

//Khai báo 4 chân kết nối ESP32 với bộ thu tín hiệu của tay PS2
#define PS2_DAT 19
#define PS2_CMD 23
#define PS2_SEL 5
#define PS2_CLK 18

//Khai báo 8 chân điều khiển động cơ qua IC MX1616H
#define PIN1M1 13
#define PIN2M1 12
#define PIN1M2 14
#define PIN2M2 27
#define PIN1M3 25
#define PIN2M3 26
#define PIN1M4 32
#define PIN2M4 33
//Xung PWM maximum (Từ 0->255)
#define PWM 200

//Tần số xung PWM, 10Khz giúp động cơ không bị rít
#define RES 8 // Resolution in bits
#define FREQ 10000 // PWM Frequency in Hz
const int deadZone = 5;

PS2X ps2x;
int error = -1;
byte type = 0;
MX1508 motor1(PIN1M1, PIN2M1, 0, 1);
MX1508 motor2(PIN1M2, PIN2M2, 2, 3);
MX1508 motor3(PIN1M3, PIN2M3, 4, 5);
MX1508 motor4(PIN1M4, PIN2M4, 6, 7);

void setup() {
  Serial.begin(115200);

  motor1.motorStop();
  motor2.motorStop();
  motor3.motorStop();
  motor4.motorStop();

  // Initialize PS2 controller
  while (error != 0) {
    delay(1000); // 1 second wait
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
    Serial.println("# Trying PS2 configuration...");
  }

  type = ps2x.readType();
  if (type == 1) Serial.println("DualShock Controller found");
}

void loop() {
  ps2x.read_gamepad(false, 0);

  int analogLUpDown = (abs(ps2x.Analog(PSS_LY) - 127) > deadZone) ? ps2x.Analog(PSS_LY) : ps2x.Analog(PSS_RY);
  int analogLLeftRight = ps2x.Analog(PSS_LX);
  int analogRLeftRight = ps2x.Analog(PSS_RX);

  bool buttonSq = ps2x.Button(PSB_SQUARE);
  bool buttonO = ps2x.Button(PSB_CIRCLE);

  if (buttonSq) {
    motor2.motorRev(PWM);
    motor3.motorRev(PWM);
  } else if (buttonO) {
    motor1.motorRev(PWM);
    motor4.motorRev(PWM);
  } else if (abs(analogLUpDown - 127) > deadZone) {
    int speed = (analogLUpDown < 127 ? 127 - analogLUpDown : analogLUpDown - 127) * (PWM / 127.0);
    speed = constrain(speed, 0, PWM);
    if (analogLUpDown < 127) {
      motor1.motorRev(speed);
      motor2.motorRev(speed);
      motor3.motorRev(speed);
      motor4.motorRev(speed);
    } else {
      motor1.motorGo(speed);
      motor2.motorGo(speed);
      motor3.motorGo(speed);
      motor4.motorGo(speed);
    }
  } else if (abs(analogLLeftRight - 127) > deadZone) {
    int speed = (analogLLeftRight < 127 ? 127 - analogLLeftRight : analogLLeftRight - 127) * (PWM / 127.0);
    speed = constrain(speed, 0, PWM);
    if (analogLLeftRight < 127) {
      motor1.motorGo(speed);
      motor2.motorRev(speed);
      motor3.motorRev(speed);
      motor4.motorGo(speed);
    } else {
      motor1.motorRev(speed);
      motor2.motorGo(speed);
      motor3.motorGo(speed);
      motor4.motorRev(speed);
    }
  } else if (abs(analogRLeftRight - 127) > deadZone) {
    int speed = (analogRLeftRight < 127 ? 127 - analogRLeftRight : analogRLeftRight - 127) * (PWM / 127.0);
    speed = constrain(speed, 0, PWM * 0.7);
    if (analogRLeftRight < 127) {
      motor1.motorGo(speed);
      motor2.motorRev(speed);
      motor3.motorGo(speed);
      motor4.motorRev(speed);
    } else {
      motor1.motorRev(speed);
      motor2.motorGo(speed);
      motor3.motorRev(speed);
      motor4.motorGo(speed);
    }
  } else {
    motor1.motorStop();
    motor2.motorStop();
    motor3.motorStop();
    motor4.motorStop();
  }
  delay(50); // Optional delay
}
