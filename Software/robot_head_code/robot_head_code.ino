#include <Wire.h>
#include <Servo.h>
#include "HUSKYLENS.h"

HUSKYLENS huskylens;
Servo SERVO_1;
Servo SERVO_2;

// --- motion tuning ---
float incrementer = 1;
const int MOVE_DELAY = 10;

int angle_1 = 107; // tilt (up/down)
int angle_2 = 107; // pan  (left/right)

// --- ranges (adjust these as needed) ---
int X_LEFT_MAX   = 130;
int X_RIGHT_MIN  = 150;
int Y_TOP_MAX    = 125;
int Y_BOTTOM_MIN = 155;

// clamp helper to keep angles in 0..214 before mapping
inline void clamp214(int &a) { if (a < 0) a = 0; if (a > 214) a = 214; }

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (!huskylens.begin(Wire)) {
    Serial.println("HUSKYLENS not found (I2C)."); while (1);
  }
  Serial.println("I2C ready. Face rec running on device.");

  SERVO_1.attach(2);
  SERVO_2.attach(3);
  SERVO_1.write(map(angle_1,0,214,0,180));
  SERVO_2.write(map(angle_2,0,214,0,180));
}

void loop() {
  // off-frame → no motion
  if (!huskylens.request()) { delay(30); return; }

  bool sawFace = false;

  while (huskylens.available()) {
    HUSKYLENSResult r = huskylens.read();
    if (r.command != COMMAND_RETURN_BLOCK || r.ID <= 0) continue;
    sawFace = true;

    int x = r.xCenter;
    int y = r.yCenter;
    Serial.print("X: "); Serial.print(x); Serial.print("  Y: "); Serial.println(y);

    if (x < X_LEFT_MAX) {                 // LEFT SIDE
      if (y < Y_TOP_MAX) {                // top-left  (DIAGONAL: up + left->center)
        angle_1 += (int)incrementer;      // tilt up
        angle_2 += (int)incrementer;      // pan toward center from left
        clamp214(angle_1); clamp214(angle_2);
        SERVO_1.write(map(angle_1,0,214,0,180));
        SERVO_2.write(map(angle_2,0,214,0,180));
        Serial.println("top left (diag)");
        delay(MOVE_DELAY);
      } else if (y < Y_BOTTOM_MIN) {      // left (PAN ONLY)
        angle_2 += (int)incrementer;
        clamp214(angle_2);
        SERVO_2.write(map(angle_2,0,214,0,180));
        Serial.println("left (pan)");
        delay(MOVE_DELAY);
      } else {                            // bottom-left (DIAGONAL: down + left->center)
        angle_1 -= (int)incrementer;      // tilt down
        angle_2 += (int)incrementer;      // pan toward center
        clamp214(angle_1); clamp214(angle_2);
        SERVO_1.write(map(angle_1,0,214,0,180));
        SERVO_2.write(map(angle_2,0,214,0,180));
        Serial.println("bottom left (diag)");
        delay(MOVE_DELAY);
      }
    }
    else if (x < X_RIGHT_MIN) {           // MIDDLE COLUMN
      if (y < Y_TOP_MAX) {                // top (TILT ONLY)
        angle_1 += (int)incrementer;
        clamp214(angle_1);
        SERVO_1.write(map(angle_1,0,214,0,180));
        Serial.println("top (tilt)");
        delay(MOVE_DELAY);
      } else if (y < Y_BOTTOM_MIN) {      // center (HOLD)
        SERVO_1.write(map(angle_1,0,214,0,180));
        SERVO_2.write(map(angle_2,0,214,0,180));
        Serial.println("middle (hold)");
      } else {                            // bottom (TILT ONLY)
        angle_1 -= (int)incrementer;
        clamp214(angle_1);
        SERVO_1.write(map(angle_1,0,214,0,180));
        Serial.println("bottom (tilt)");
        delay(MOVE_DELAY);
      }
    }
    else {                                 // RIGHT SIDE
      if (y < Y_TOP_MAX) {                 // top-right (DIAGONAL: up + right->center)
        angle_1 += (int)incrementer;       // tilt up
        angle_2 -= (int)incrementer;       // pan toward center from right
        clamp214(angle_1); clamp214(angle_2);
        SERVO_1.write(map(angle_1,0,214,0,180));
        SERVO_2.write(map(angle_2,0,214,0,180));
        Serial.println("top right (diag)");
        delay(MOVE_DELAY);
      } else if (y < Y_BOTTOM_MIN) {       // right (PAN ONLY)
        angle_2 -= (int)incrementer;
        clamp214(angle_2);
        SERVO_2.write(map(angle_2,0,214,0,180));
        Serial.println("right (pan)");
        delay(MOVE_DELAY);
      } else {                             // bottom-right (DIAGONAL: down + right->center)
        angle_1 -= (int)incrementer;       // tilt down
        angle_2 += (int)incrementer;       // pan toward center
        clamp214(angle_1); clamp214(angle_2);
        SERVO_1.write(map(angle_1,0,214,0,180));
        SERVO_2.write(map(angle_2,0,214,0,180));
        Serial.println("bottom right (diag)");
        delay(MOVE_DELAY);
      }
    }
  }

  // no face this frame → do nothing
  if (!sawFace) { /* chill */ }

  delay(30);
}
