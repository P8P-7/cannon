#include <DynamixelSerial.h>
#include <Timer.h>

#define PIN_SERVO_DIRECTION 0x02
#define PIN_VALVE 0x05
#define PIN_COIL 0x06
#define PIN_VALVE_BUTTON 0x07
#define PIN_COIL_BUTTON 0x08
#define PIN_JOYSTICK_X A0
#define PIN_JOYSTICK_Y A1

#define TURN_SERVO 0x02
#define TILT_SERVO 0x01

#define JOYSTICK_DEADZONE_MIN 450
#define JOYSTICK_DEADZONE_MAX 562

#define DELAY_OPEN_CLOSE_VALVE 175
#define DELAY_CLOSE_VALVE_FIRE 200
#define DELAY_FIRE_STOP_FIRE 370
#define DELAY_RE_AUTO_FIRE 380

#define RIGHT 1

bool valveOpen = false;
bool firing = false;

bool autoFiring = false;
Timer autoFireTimer;

void joystickToServo(double joystickValue, int servo) {
    if (joystickValue >= JOYSTICK_DEADZONE_MAX) {
        Dynamixel.turn(servo, RIGHT, map(joystickValue, 512, 1023, 0, 1023));
    } else if (joystickValue < JOYSTICK_DEADZONE_MIN) {
        Dynamixel.turn(servo, LEFT, map(joystickValue, 512, 0, 0, 1023));
    } else {
        Dynamixel.turn(servo, RIGHT, 0);
    }
}

void openValve() {
    if (!valveOpen) {
        digitalWrite(PIN_VALVE, HIGH);
        valveOpen = true;
    }
}

void closeValve() {
    if (valveOpen) {
        digitalWrite(PIN_VALVE, LOW);
        valveOpen = false;
    }
}

void fire() {
    if (!firing) {
        digitalWrite(PIN_COIL, HIGH);
        firing = true;
    }
}

void stopFire() {
    if (firing) {
        digitalWrite(PIN_COIL, LOW);
        firing = false;
    }
}

void autoFire() {
    autoFireTimer.after(DELAY_OPEN_CLOSE_VALVE, &closeValve);
    autoFireTimer.after(DELAY_CLOSE_VALVE_FIRE, &fire);
    autoFireTimer.after(DELAY_FIRE_STOP_FIRE, &stopFire);
    autoFireTimer.after(DELAY_RE_AUTO_FIRE, &stopAutoFire);
}

void stopAutoFire() {
    autoFiring = false;
}

void setup() {
    pinMode(PIN_VALVE, OUTPUT);
    pinMode(PIN_COIL, OUTPUT);
    
    pinMode(PIN_JOYSTICK_X, INPUT);
    pinMode(PIN_JOYSTICK_Y, INPUT);
    pinMode(PIN_VALVE_BUTTON, INPUT);
    pinMode(PIN_COIL_BUTTON, INPUT);
  
    Dynamixel.begin(1000000, PIN_SERVO_DIRECTION);
    Dynamixel.setEndless(TURN_SERVO, ON);
    Dynamixel.setEndless(TILT_SERVO, ON);

    for (int i = 0; i < 14; ++i) {
        digitalWrite(i, LOW);
    }
}

void loop() {  
    joystickToServo(analogRead(PIN_JOYSTICK_X), TURN_SERVO);
    joystickToServo(analogRead(PIN_JOYSTICK_Y), TILT_SERVO);
    
    if (autoFiring) {
        autoFireTimer.update();
        return;
    }

    if (digitalRead(PIN_VALVE_BUTTON) == HIGH && digitalRead(PIN_COIL_BUTTON)) {
        autoFire();
        autoFiring = true;
    }

    if (digitalRead(PIN_VALVE_BUTTON) == HIGH) {
        openValve(); 
    } else {
        closeValve();
    }

    if (digitalRead(PIN_COIL_BUTTON) == HIGH) {
        fire();
    } else {
        stopFire();
    }
} 
